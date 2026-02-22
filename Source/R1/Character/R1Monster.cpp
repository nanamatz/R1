#include "Character/R1Monster.h"
#include "R1Player.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"

#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "AbilitySystem/Attribute/MonsterAttributeSet.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"

#include "AI/R1AIController.h"
#include "BrainComponent.h"
#include "Animation/R1AnimInstance.h"

#include "UI/R1HpBarWidget.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "Map/DungeonManager.h"
#include "DataTable/CharacterStatsRow.h"


AR1Monster::AR1Monster()
{
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));

	AbilitySystemComponent = CreateDefaultSubobject<UR1AbilitySystemComponent>("AbilitySystemComponent");
	MonsterAttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>("MonsterAttributeSet");
	CoreAttributeSet = CreateDefaultSubobject<UR1AttributeSet>("CoreAttributeSet");


	HpBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HpBarComponent->SetupAttachment(GetRootComponent());

	ConstructorHelpers::FClassFinder<UUserWidget> HealthBarWidgetClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Blueprints/UI/WBP_HpBar.WBP_HpBar_C'"));
	if (HealthBarWidgetClass.Succeeded())
	{
		HpBarComponent->SetWidgetClass(HealthBarWidgetClass.Class);
		HpBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
		HpBarComponent->SetDrawAtDesiredSize(true);
		HpBarComponent->SetRelativeLocation(FVector(0, 0, 120));
	}

	Tags.Add(FName("Enemy"));
}

void AR1Monster::BeginPlay()
{
	Super::BeginPlay();

	InitAbilitySystem();
	
	InitAttributes();
	
	AR1Monster* Monster = Cast<AR1Monster>(this);

	if (Monster)
	{
		Monster->OnHpChanged.AddDynamic(this, &AR1Monster::RefreshHpBar);
	}
	AggroRange = AbilitySystemComponent->GetNumericAttribute(MonsterAttributeSet->GetAggroRangeAttribute());
	
	RefreshHpBar(1.f);
}

void AR1Monster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AR1Monster::InitAbilitySystem()
{
	Super::InitAbilitySystem();
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void AR1Monster::RefreshHpBar(float Ratio)
{
	if (HpBarComponent && CoreAttributeSet)
	{
		UR1HpBarWidget* HpBar = Cast<UR1HpBarWidget>(HpBarComponent->GetUserWidgetObject());
		if (HpBar)
		{
			HpBar->SetHpRatio(Ratio);
		}
	}
}

void AR1Monster::ActivateAbility(FGameplayTag AbilityTag)
{
	AbilitySystemComponent->ActivateAbility(AbilityTag);
}

void AR1Monster::OnDead(const TObjectPtr<class AR1Character> Attacker)
{
	Super::OnDead(Attacker);

	if (Attacker == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("오류: Attacker가 NULL입니다! 데미지 GE 컨텍스트에서 Instigator를 세팅하지 않았습니다."));
		return;
	}

	AR1AIController* AIC = Cast<AR1AIController>(GetController());

	if (AIC)
	{
		// 리소스 낭비를 막기 위해 AI 로직(비헤이비어 트리) 중단
		AIC->StopMovement();
		AIC->BrainComponent->StopLogic("Dead");
	}

	if (DeathAnimMontage)
	{
		UAnimInstance* AnimInstance = Cast<UAnimInstance>(GetMesh()->GetAnimInstance());
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(DeathAnimMontage);
		}
	}

	if (HpBarComponent)
	{
		HpBarComponent->SetHiddenInGame(true);
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Attacker);

	// 2. 내(몬스터) ASC도 필요합니다.
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponent();

	if (TargetASC && SourceASC && XpEffect) // RewardExpGEClass는 블루프린트에서 할당해둔 GE 클래스
	{
		// 3. GE Spec(명세서)을 만듭니다. (내가 저격수고, 타겟에게 쏠 총알을 만드는 과정)
		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
		ContextHandle.AddInstigator(this, this); // 이 경험치의 출처는 나(몬스터)야!

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(XpEffect, 1.0f, ContextHandle);

		// 4. 플레이어에게 GE를 적용합니다!
		TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}

	//SetLifeSpan(5.f); // 5초 뒤에 사라지도록 설정 (죽은 시점부터 사라질 때까지 시간 벌기)
}

void AR1Monster::InitializeWithManager(ADungeonManager* InManager)
{
	if (!IsValid(InManager)) return;

	// 1. 주입받은 지휘관의 명부에 내 이름을 올립니다.
	InManager->RegisterMonster(this);
	//// 2. 내가 죽을 때 지휘관의 명부에서 빠지도록 이벤트를 묶어줍니다.

	this->OnDeadDelegate.AddDynamic(InManager, &ADungeonManager::UnregisterMonster);

}

void AR1Monster::InitAttributes()
{
	// 1. 부모 함수 호출 (체력, 공격력 세팅)
	Super::InitAttributes();

	// 2. 몬스터 전용 스탯(경험치 드롭량, 어그로 범위) 세팅
	if (!AbilitySystemComponent || !CharacterStatTable || !MonsterInitStatEffectClass) return;

	FR1CharacterStatsRow* StatData = CharacterStatTable->FindRow<FR1CharacterStatsRow>(CharacterRowName, TEXT("InitMonsterAttributes"));
	if (StatData)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(MonsterInitStatEffectClass, 1.0f, Context);

		if (SpecHandle.IsValid())
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Attribute.Xp")), StatData->Xp);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Attribute.AggroRange")), StatData->AggroRange);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Attribute.AttackAngle")), StatData->AttackAngle);

			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}
