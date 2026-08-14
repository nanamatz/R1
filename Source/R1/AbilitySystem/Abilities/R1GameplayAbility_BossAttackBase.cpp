#include "AbilitySystem/Abilities/R1GameplayAbility_BossAttackBase.h"
#include "Data/R1TelegraphData.h"
#include "Object/R1TelegraphActor.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "R1GameplayTags.h"
#include "System/R1GameInstance.h"

UR1GameplayAbility_BossAttackBase::UR1GameplayAbility_BossAttackBase()
{
	AttackEventTag = R1GameplayTags::Event_Montage_Attack;
}

void UR1GameplayAbility_BossAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 1. Commit Ability (Check Cost and Cooldown)
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. Spawn Telegraph Actor
	if (TelegraphData && TelegraphActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = AvatarActor;
		SpawnParams.Instigator = Cast<APawn>(AvatarActor);

		FRotator SpawnRotation = FRotator(0.0f, AvatarActor->GetActorRotation().Yaw, 0.0f);
		FVector SpawnLocation = AvatarActor->GetActorLocation();

		AR1TelegraphActor* TelegraphActor = GetWorld()->SpawnActor<AR1TelegraphActor>(TelegraphActorClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (TelegraphActor)
		{
			TelegraphActor->InitializeTelegraph(TelegraphData);
		}
	}

	// 3. Play Montage
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("AttackMontage"), MontageToPlay);
	if (PlayMontageTask)
	{
		PlayMontageTask->OnCompleted.AddDynamic(this, &UR1GameplayAbility_BossAttackBase::OnMontageEnded);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UR1GameplayAbility_BossAttackBase::OnMontageEnded);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UR1GameplayAbility_BossAttackBase::OnMontageEnded);
		PlayMontageTask->ReadyForActivation();
	}

	// 몽타주 재생이 즉시 실패하면 OnCancelled → OnMontageEnded → EndAbility가 위 호출 안에서
	// 동기 실행될 수 있다. 종료된 인스턴스에 AddDynamic하면 ensure가 발생하므로 중단한다.
	if (!IsActive())
	{
		return;
	}

	// 4. Wait for Gameplay Event
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

void UR1GameplayAbility_BossAttackBase::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (SkillID.IsNone()) return;

	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid()) return;

	UWorld* World = ActorInfo->AvatarActor->GetWorld();
	if (!World) return;

	if (UR1GameInstance* GI = Cast<UR1GameInstance>(World->GetGameInstance()))
	{
		if (const FSkillDataRow* Data = GI->GetSkillData(SkillID))
		{
			CachedDamage = Data->Damage;
			CachedManaCost = Data->ManaCost;
			CachedCooldown = Data->Cooldown;
			CachedRange = Data->Range;
		}
	}
}

bool UR1GameplayAbility_BossAttackBase::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags)) return false;

	if (CachedManaCost <= 0.0f) return true;

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		// Bosses might use a different AttributeSet or no mana at all. 
		// For now, let's follow the Player pattern but be careful.
		// If we want bosses to have costs, we need to ensure they have the Mana attribute.
	}

	return true;
}

void UR1GameplayAbility_BossAttackBase::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (CostEffectClass && CachedManaCost > 0.0f)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CostEffectClass, GetAbilityLevel(Handle, ActorInfo));
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Cost, -CachedManaCost);
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}
}

void UR1GameplayAbility_BossAttackBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	// 쿨다운 GE의 지속시간을 DT_BossSkillData의 Cooldown 값으로 주입한다. (Super 미호출 — GE 고정 duration 대체)
	if (CooldownGameplayEffectClass && CachedCooldown > 0.0f)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CooldownGameplayEffectClass, GetAbilityLevel(Handle, ActorInfo));
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Cooldown, CachedCooldown);
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}
}
