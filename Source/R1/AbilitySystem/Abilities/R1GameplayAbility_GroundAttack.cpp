#include "AbilitySystem/Abilities/R1GameplayAbility_GroundAttack.h"
#include "R1LogChannels.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/R1Player.h"
#include "Data/R1TelegraphData.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "R1GameplayTags.h"
#include "NiagaraSystem.h"

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

	if (EffectSpecHandle.IsValid())
	{
		EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Magnitude, CachedDamage);
	}

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

			// [VFX] 피해를 준 플레이어 위치에 무기 임팩트 큐 실행 (어빌리티 지정 VFX를 SourceObject로 전달)
			FGameplayCueParameters CueParams;
			CueParams.SourceObject = HitImpactVFX;
			CueParams.Instigator = AvatarActor;
			CueParams.Location = OverlappedActor->GetActorLocation() + FVector(0, 0, 50.0f); // 명치 높이 보정
			CueParams.Normal = (AttackLocation - OverlappedActor->GetActorLocation()).GetSafeNormal();
			SourceASC->ExecuteGameplayCue(R1GameplayTags::GameplayCue_Weapon_Impact, CueParams);
		}
	}

	UE_LOG(LogR1, Log, TEXT("GroundAttack: Hit %d actors"), OverlappedActors.Num());
}
