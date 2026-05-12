#include "AbilitySystem/Abilities/R1GameplayAbility_WaveAttack.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Character/R1Player.h"
#include "Data/R1TelegraphData.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "R1GameplayTags.h"
#include "Particles/ParticleSystemComponent.h"
#include "Character/Boss/BossBaby.h"

void UR1GameplayAbility_WaveAttack::OnAttackEventReceived(FGameplayEventData Payload)
{
	ABossBaby* AvatarActor = Cast<ABossBaby>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	if (!AvatarActor || !SourceASC || !TelegraphData || !DamageEffect)
	{
		return;
	}

	FVector BossLoc = AvatarActor->GetActorLocation();
	FVector BossForward = AvatarActor->GetActorForwardVector();

	float Length = TelegraphData->TelegraphSize.X;
	float Width = TelegraphData->TelegraphSize.Y;

	// The Box Overlap center should be Half-Length in front of the boss
	FVector BoxCenter = BossLoc + (BossForward * (Length / 2.0f));
	// Box Extent is half-size in each dimension
	FVector BoxExtent = FVector(Length / 2.0f, Width / 2.0f, 100.0f); 

	// Create Effect Spec
	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);

	if (EffectSpecHandle.IsValid())
	{
		EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Magnitude, CachedDamage);
	}

	if (!EffectSpecHandle.IsValid())
	{
		return;
	}

	// Spawn Projectile
	FVector MuzzleLocation = AvatarActor->GetMesh()->GetSocketLocation(AvatarActor->MuzzleSocketName);

	// Direction: For now, use character forward. In a real game, you might want to aim at the target.
	FRotator MuzzleRotation = AvatarActor->GetActorRotation();

	if (WaveEffect)
	{
		// 스폰 후 컴포넌트 포인터를 받아옵니다.
		UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			WaveEffect,
			MuzzleLocation,
			MuzzleRotation
		);

		if (ParticleComp)
		{
			FVector BeamEndLocation = MuzzleLocation + (BossForward * Length);
			ParticleComp->SetVectorParameter(FName("BeamTarget"), BeamEndLocation);
		}

		// Overlap Check (Box)
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(AvatarActor);

		TArray<AActor*> OverlappedActors;
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

		UKismetSystemLibrary::BoxOverlapActors(GetWorld(), BoxCenter, BoxExtent, ObjectTypes, AR1Player::StaticClass(), IgnoreActors, OverlappedActors);

		for (AActor* OverlappedActor : OverlappedActors)
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlappedActor);
			if (TargetASC)
			{
				SourceASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("WaveAttack (Laser): Hit %d actors"), OverlappedActors.Num());
	}
}
