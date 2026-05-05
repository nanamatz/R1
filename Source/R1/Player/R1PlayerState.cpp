


#include "Player/R1PlayerState.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "System/R1SaveSystem.h"
#include "System/R1MetaSaveGame.h"
#include "DataTable/R1MetaUpgradeData.h"
#include "Player/R1RunUpgradeComponent.h"
#include "GameplayEffectExtension.h"

AR1PlayerState::AR1PlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UR1AbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	PlayerAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
	CoreAttributeSet = CreateDefaultSubobject<UR1AttributeSet>(TEXT("CoreAttributeSet"));

	RunUpgradeComponent = CreateDefaultSubobject<UR1RunUpgradeComponent>(TEXT("RunUpgradeComponent"));
}


UAbilitySystemComponent* AR1PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UR1AbilitySystemComponent* AR1PlayerState::GetR1AbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UPlayerAttributeSet* AR1PlayerState::GetPlayerAttributeSet() const
{
	return PlayerAttributeSet;
}

UR1AttributeSet* AR1PlayerState::GetCommonAttributeSet() const
{
	return CoreAttributeSet;
}

void AR1PlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent && PlayerAttributeSet)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributeSet->GetLevelAttribute()).AddUObject(this, &AR1PlayerState::OnLevelChanged);
	}

	if (RunUpgradeComponent)
	{
		RunUpgradeComponent->Reset();
	}
}

float AR1PlayerState::GetCurrentExpRatio() const
{
	if (AbilitySystemComponent)
	{
		float Exp = AbilitySystemComponent->GetNumericAttribute(PlayerAttributeSet->GetExpAttribute());
		float MaxExp = AbilitySystemComponent->GetNumericAttribute(PlayerAttributeSet->GetMaxExpAttribute());
		if (MaxExp > 0)
		{
			return Exp / MaxExp;
		}
	}
	return 0.f;
}

void AR1PlayerState::ApplyMetaUpgrades()
{
	if (!AbilitySystemComponent || !MetaUpgradeEffectClass || !MetaUpgradeDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerState] 메타 업그레이드 적용 실패: ASC, GE 클래스 또는 데이터 테이블이 누락되었습니다."));
		return;
	}

	// 1. 세이브 시스템에서 영구 금고(MetaSave) 데이터를 가져옵니다.
	UR1SaveSystem* SaveSystem = GetGameInstance()->GetSubsystem<UR1SaveSystem>();
	if (!SaveSystem) return;

	UR1MetaSaveGame* MetaSave = SaveSystem->LoadMetaProgression();
	if (!MetaSave || MetaSave->InvestedUpgrades.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("[PlayerState] 투자된 메타 스킬이 없습니다. 기본 스탯으로 시작합니다."));
		return;
	}

	if (UPlayerAttributeSet* PlayerAttr = GetPlayerAttributeSet())
	{
		// 1. 레벨 동기화
		PlayerAttr->SetLevel(MetaSave->PlayerMetaLevel);

		// 2. [수정] 0으로 초기화하지 않고, 세이브된 메타 경험치를 그대로 주입
		PlayerAttr->SetExp(MetaSave->CurrentMetaExp);

		// 3. 현재 레벨에 맞는 MaxExp 계산
		if (PlayerStatTable)
		{
			FRealCurve* MaxExpCurve = PlayerStatTable->FindCurve(FName("MaxExp"), TEXT(""));
			if (MaxExpCurve)
			{
				float NewMaxExp = MaxExpCurve->Eval(MetaSave->PlayerMetaLevel);
				PlayerAttr->SetMaxExp(NewMaxExp);

				// UI 갱신을 위한 방송 (현재 경험치 / 새로운 MaxExp 비율)
				float Ratio = (NewMaxExp > 0) ? (MetaSave->CurrentMetaExp / NewMaxExp) : 0.0f;
				OnExpChanged.Broadcast(Ratio);

				UE_LOG(LogTemp, Warning, TEXT("[Meta Load] 레벨: %d, 경험치: %f / %f (동기화 완료)"),
					MetaSave->PlayerMetaLevel, MetaSave->CurrentMetaExp, NewMaxExp);
			}
		}
	}

	// 2. Gameplay Effect 스펙(Spec)을 생성합니다.
	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(MetaUpgradeEffectClass, 1.0f, ContextHandle);

	if (!SpecHandle.IsValid()) return;

	// 3. 데이터 테이블의 모든 행(Row)을 가져옵니다.
	TArray<FR1MetaUpgradeData*> AllUpgrades;
	MetaUpgradeDataTable->GetAllRows<FR1MetaUpgradeData>(TEXT("MetaUpgradeContext"), AllUpgrades);

	// [Fix] 초기화: 모든 태그에 대해 0.0f로 먼저 설정하여, 투자하지 않은 항목에 대해 GAS 에러가 발생하는 것을 방지합니다.
	for (FR1MetaUpgradeData* UpgradeData : AllUpgrades)
	{
		if (UpgradeData)
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(UpgradeData->UpgradeTag, 0.0f);
		}
	}

	// 4. 유저가 투자한 내역을 순회하며 정확한 수치를 계산합니다.
	for (const auto& Pair : MetaSave->InvestedUpgrades)
	{
		FGameplayTag UpgradeTag = Pair.Key;
		int32 InvestedLevel = Pair.Value;

		if (InvestedLevel <= 0) continue;

		// 데이터 테이블에서 이 태그와 일치하는 설정을 찾습니다.
		for (FR1MetaUpgradeData* UpgradeData : AllUpgrades)
		{
			if (UpgradeData && UpgradeData->UpgradeTag == UpgradeTag)
			{
				// 인덱스 안전 검사 (1레벨은 인덱스 0, 5레벨은 인덱스 4)
				int32 TargetIndex = FMath::Clamp(InvestedLevel - 1, 0, UpgradeData->ValuesPerLevel.Num() - 1);

				if (UpgradeData->ValuesPerLevel.IsValidIndex(TargetIndex))
				{
					float BuffValue = UpgradeData->ValuesPerLevel[TargetIndex];

					// 🌟 [핵심] SetByCaller를 이용해 태그에 해당하는 수치를 GE에 주입합니다!
					SpecHandle.Data.Get()->SetSetByCallerMagnitude(UpgradeTag, BuffValue);

					UE_LOG(LogTemp, Warning, TEXT("[PlayerState] 메타 업그레이드 장전: %s (+%f)"), *UpgradeTag.ToString(), BuffValue);
				}
				break; // 찾았으니 다음 투자 내역으로 이동
			}
		}
	}

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	if (UR1AttributeSet* CoreAttr = GetCommonAttributeSet())
	{
		AbilitySystemComponent->SetNumericAttributeBase(CoreAttr->GetHealthAttribute(), CoreAttr->GetMaxHealth());
	}

	if (UPlayerAttributeSet* PlayerAttr = GetPlayerAttributeSet())
	{
		AbilitySystemComponent->SetNumericAttributeBase(PlayerAttr->GetManaAttribute(), PlayerAttr->GetMaxMana());
	}
}

void AR1PlayerState::OnLevelChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue > Data.OldValue)
	{
		if (RunUpgradeComponent)
		{
			RunUpgradeComponent->AddPoints(5);
		}
	}
}

