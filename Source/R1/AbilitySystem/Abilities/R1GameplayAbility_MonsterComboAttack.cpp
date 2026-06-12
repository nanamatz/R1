
#include "AbilitySystem/Abilities/R1GameplayAbility_MonsterComboAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Character/R1Character.h"
#include "Character/R1Player.h"
#include "AbilitySystemComponent.h"
#include "R1GameplayTags.h"
#include "Library/R1AbilitySystemLibrary.h"

UR1GameplayAbility_MonsterComboAttack::UR1GameplayAbility_MonsterComboAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UR1GameplayAbility_MonsterComboAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AR1Character* Attacker = Cast<AR1Character>(ActorInfo->AvatarActor);

	// Combo Section Cycling
	FName StartSectionName = (ComboIndex == 0) ? FName("Combo1") : FName("Combo2");

	if (PlayAttackMontageAndWaitForEvent(Attacker, AttackEventTag, StartSectionName))
	{
		ComboIndex = (ComboIndex + 1) % 2;
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UR1GameplayAbility_MonsterComboAttack::OnMontageEnded()
{
	if (CurrentActorInfo == nullptr || CurrentActorInfo->AvatarActor.IsValid() == false)
	{
		return;
	}

	AR1Character* Attacker = Cast<AR1Character>(CurrentActorInfo->AvatarActor.Get());
	if (Attacker)
	{
		if (Attacker->GetCreatureState() != ECreatureState::Dead)
		{
			Attacker->SetCreatureState(ECreatureState::Moving);
		}
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UR1GameplayAbility_MonsterComboAttack::OnAttackEventReceived(FGameplayEventData Payload)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	AR1Character* SourceCharacter = Cast<AR1Character>(AvatarActor);

	if (!SourceCharacter) return;

	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, SourceCharacter->GetActorLocation());
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	if (DamageEffect && SourceASC)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);
		if (EffectSpecHandle.IsValid() && EffectSpecHandle.Data.IsValid())
		{
			UR1AbilitySystemLibrary::ApplySectorDamageToPlayers(EffectSpecHandle, SourceCharacter, SourceASC);
		}
	}
}
