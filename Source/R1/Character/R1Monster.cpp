


#include "Character/R1Monster.h"
#include "R1Player.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "AbilitySystem/Attribute/R1MonsterAttributeSet.h"

#include "AI/R1AIController.h"
#include "BrainComponent.h"
#include "Animation/R1AnimInstance.h"

#include "UI/R1HpBarWidget.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"


AR1Monster::AR1Monster()
{
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));

	AbilitySystemComponent = CreateDefaultSubobject<UR1AbilitySystemComponent>("AbilitySystemComponent");
	AttributeSet = CreateDefaultSubobject<UR1MonsterAttributeSet>("MonsterAttributeSet");


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
	
	RefreshHpBar();

	InitAttributes();
}

void AR1Monster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RefreshHpBar();
}

void AR1Monster::InitAbilitySystem()
{
	Super::InitAbilitySystem();
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

//void AR1Monster::DefaultAttack()
//{
//	//TArray<FHitResult> HitResults;
//
//	//FCollisionQueryParams Params;
//	//Params.AddIgnoredActor(this);
//
//	//FVector Start = GetActorLocation();
//	//FVector End = GetActorLocation() + GetActorForwardVector() * 100.f;
//	//float Range = 50.f;
//
//	//bool bHit = GetWorld()->SweepMultiByChannel(
//	//	HitResults,
//	//	Start,
//	//	End,
//	//	FQuat::Identity,
//	//	ECC_GameTraceChannel1,
//	//	FCollisionShape::MakeSphere(Range),
//	//	Params
//	//);
//
//	//DrawDebugSphere(GetWorld(), Start, 50.f, 16, FColor::Green, false, 1.f);
//	//DrawDebugSphere(GetWorld(), End, 50.f, 16, FColor::Blue, false, 1.f);
//
//	//for(const FHitResult& HitResult : HitResults)
//	//{
//	//	AR1Player* HitCharacter = Cast<AR1Player>(HitResult.GetActor());
//
//	//	if (HitCharacter)
//	//	{
//	//		HitCharacter->OnDamaged(10, this);
//	//	}
//	//}
//
//}

void AR1Monster::RefreshHpBar()
{
	if (HpBarComponent && AttributeSet)
	{
		float Hp = AttributeSet->GetHealth();
		float MaxHp = AttributeSet->GetMaxHealth();

		float Ratio = static_cast<float>(Hp) / MaxHp;
		UR1HpBarWidget* HpBar = Cast<UR1HpBarWidget>(HpBarComponent->GetUserWidgetObject());
		HpBar->SetHpRatio(Ratio);
	}
}

void AR1Monster::ActivateAbility(FGameplayTag AbilityTag)
{
	AbilitySystemComponent->ActivateAbility(AbilityTag);
}

void AR1Monster::OnDead(const TObjectPtr<class AR1Character> Attacker)
{
	Super::OnDead(Attacker);

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
	//SetLifeSpan(5.0f);
}


//void AR1Monster::SetCreatureState(ECreatureState InState)
//{
//	CreatureState = InState;
//}
//
//ECreatureState AR1Monster::GetCreatureState()
//{
//	return CreatureState;
//}
