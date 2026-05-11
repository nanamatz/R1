#include "AbilitySystem/Abilities/R1GameplayAbility_GroundAttack.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/R1Player.h"
#include "Data/R1TelegraphData.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

void UR1GameplayAbility_GroundAttack::OnAttackEventReceived(FGameplayEventData Payload)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	if (!AvatarActor || !SourceASC || !TelegraphData || !DamageEffect)
	{
		return;
	}

	FVector AttackLocation = AvatarActor->GetActorLocation();
	float Radius = TelegraphData->TelegraphSize.X;

	// Create Effect Spec
	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);

	if (!EffectSpecHandle.IsValid())
	{
		return;
	}

	// Overlap Check
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(AvatarActor);

	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), AttackLocation, Radius, ObjectTypes, AR1Player::StaticClass(), IgnoreActors, OverlappedActors);

	for (AActor* OverlappedActor : OverlappedActors)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlappedActor);
		if (TargetASC)
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GroundAttack: Hit %d actors"), OverlappedActors.Num());
}
