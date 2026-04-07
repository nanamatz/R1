


#include "Player/R1PlayerState.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "System/R1SaveSystem.h"
#include "System/R1MetaSaveGame.h"
#include "DataTable/R1MetaUpgradeData.h"

AR1PlayerState::AR1PlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UR1AbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	PlayerAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
	CoreAttributeSet = CreateDefaultSubobject<UR1AttributeSet>(TEXT("CoreAttributeSet"));

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

	// 2. Gameplay Effect 스펙(Spec)을 생성합니다.
	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(MetaUpgradeEffectClass, 1.0f, ContextHandle);

	if (!SpecHandle.IsValid()) return;

	// 3. 데이터 테이블의 모든 행(Row)을 가져옵니다.
	TArray<FR1MetaUpgradeData*> AllUpgrades;
	MetaUpgradeDataTable->GetAllRows<FR1MetaUpgradeData>(TEXT("MetaUpgradeContext"), AllUpgrades);

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

	// 5. 완성된 스펙(모든 버프 수치가 담긴 GE)을 플레이어 자신에게 적용합니다!
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
