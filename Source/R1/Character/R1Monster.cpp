


#include "Character/R1Monster.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "R1Player.h"
#include "AbilitySystem/Attribute/R1MonsterAttributeSet.h"

AR1Monster::AR1Monster()
{
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));

	AbilitySystemComponent = CreateDefaultSubobject<UR1AbilitySystemComponent>("AbilitySystemComponent");
	AttributeSet = CreateDefaultSubobject<UR1MonsterAttributeSet>("MonsterAttributeSet");

	Tags.Add(FName("Enemy"));
}

void AR1Monster::BeginPlay()
{
	Super::BeginPlay();

	InitAbilitySystem();
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

void AR1Monster::DefaultAttack()
{
	TArray<FHitResult> HitResults;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FVector Start = GetActorLocation();
	FVector End = GetActorLocation() + GetActorForwardVector() * 100.f;
	float Range = 50.f;

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_GameTraceChannel1,
		FCollisionShape::MakeSphere(Range),
		Params
	);

	DrawDebugSphere(GetWorld(), Start, 50.f, 16, FColor::Green, false, 1.f);
	DrawDebugSphere(GetWorld(), End, 50.f, 16, FColor::Blue, false, 1.f);
	DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 1.f);

	for(const FHitResult& HitResult : HitResults)
	{
		AR1Player* HitCharacter = Cast<AR1Player>(HitResult.GetActor());

		if (HitCharacter)
		{
			HitCharacter->OnDamaged(10, this);
		}
	}

}

void AR1Monster::ActivateAbility(FGameplayTag AbilityTag)
{
	AbilitySystemComponent->ActivateAbility(AbilityTag);
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
