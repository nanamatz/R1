#include "AbilitySystem/Abilities/R1GameplayAbility_BossAttackBase.h"
#include "Data/R1TelegraphData.h"
#include "Object/R1TelegraphActor.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "R1GameplayTags.h"

UR1GameplayAbility_BossAttackBase::UR1GameplayAbility_BossAttackBase()
{
	AttackEventTag = R1GameplayTags::Event_Montage_Attack;
}

void UR1GameplayAbility_BossAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. Spawn Telegraph Actor
	if (TelegraphData && TelegraphActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = AvatarActor;
		SpawnParams.Instigator = Cast<APawn>(AvatarActor);

		AR1TelegraphActor* TelegraphActor = GetWorld()->SpawnActor<AR1TelegraphActor>(TelegraphActorClass, AvatarActor->GetActorLocation(), AvatarActor->GetActorRotation(), SpawnParams);
		if (TelegraphActor)
		{
			TelegraphActor->InitializeTelegraph(TelegraphData);
		}
	}

	// 2. Play Montage
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("AttackMontage"), MontageToPlay);
	if (PlayMontageTask)
	{
		PlayMontageTask->OnCompleted.AddDynamic(this, &UR1GameplayAbility_BossAttackBase::OnMontageEnded);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UR1GameplayAbility_BossAttackBase::OnMontageEnded);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UR1GameplayAbility_BossAttackBase::OnMontageEnded);
		PlayMontageTask->ReadyForActivation();
	}

	// 3. Wait for Gameplay Event
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AttackEventTag);
	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &UR1GameplayAbility_BossAttackBase::OnAttackEventReceived);
		WaitEventTask->ReadyForActivation();
	}
}

void UR1GameplayAbility_BossAttackBase::OnAttackEventReceived(FGameplayEventData Payload)
{
	// Subclasses implement this
}

void UR1GameplayAbility_BossAttackBase::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
