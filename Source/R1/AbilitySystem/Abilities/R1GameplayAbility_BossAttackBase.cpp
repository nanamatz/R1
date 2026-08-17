#include "AbilitySystem/Abilities/R1GameplayAbility_BossAttackBase.h"
#include "Data/R1TelegraphData.h"
#include "Object/R1TelegraphActor.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "R1GameplayTags.h"
#include "R1LogChannels.h"
#include "System/R1GameInstance.h"

UR1GameplayAbility_BossAttackBase::UR1GameplayAbility_BossAttackBase()
{
	// ⚠️ 엔진 기본값은 InstancedPerExecution이다(GameplayAbility.cpp:81).
	// 그 정책에서는 OnAvatarSet이 CDO에서 돌고, 발동마다 클래스 기본값으로 새 인스턴스가
	// 만들어지므로 CachedDamage/CachedCooldown/CachedRange가 전부 0인 인스턴스가 실행된다.
	// (쿨다운이 영영 안 걸리고, SetByCaller 데미지도 0이 된다.)
	// 이 계열은 OnAvatarSet 캐시에 의존하므로 액터당 인스턴스가 필수다.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

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
		bWaitingForAttackEvent = true;
		bAttackEventReceived = false;

		WaitEventTask->EventReceived.AddDynamic(this, &UR1GameplayAbility_BossAttackBase::HandleAttackEventReceived);
		WaitEventTask->ReadyForActivation();
	}
}

void UR1GameplayAbility_BossAttackBase::HandleAttackEventReceived(FGameplayEventData Payload)
{
	bAttackEventReceived = true;
	OnAttackEventReceived(Payload);
}

void UR1GameplayAbility_BossAttackBase::OnAttackEventReceived(FGameplayEventData Payload)
{
	// Subclasses implement this
}

void UR1GameplayAbility_BossAttackBase::OnMontageEnded()
{
	// 몽타주는 끝났는데 공격 노티파이가 한 번도 안 왔다면, 이 스킬은 아무 일도 하지 않았다.
	// 원래 조용히 실패하는 경로라 (데미지 없음/이동 없음/로그 없음) 여기서 이름을 찍어준다.
	// 흔한 원인: 몽타주에 노티파이가 없음, 노티파이의 태그가 AttackEventTag와 불일치,
	// 몽타주 슬롯이 ABP에 없어 재생 자체가 실패.
	if (bWaitingForAttackEvent && !bAttackEventReceived)
	{
		UE_LOG(LogR1, Warning,
			TEXT("[%s] montage ended without ever receiving '%s'. Check the montage has an AnimNotify sending that exact tag (note: the registered string, not the C++ symbol name)."),
			*GetName(), *AttackEventTag.ToString());
	}

	bWaitingForAttackEvent = false;

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UR1GameplayAbility_BossAttackBase::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (SkillID.IsNone())
	{
		// SkillID가 비면 DataTable 조회를 아예 안 하므로 Damage/Cooldown이 0으로 남는다.
		UE_LOG(LogR1, Warning, TEXT("[%s] SkillID is empty — damage and cooldown stay 0"), *GetName());
		return;
	}

	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid()) return;

	UWorld* World = ActorInfo->AvatarActor->GetWorld();
	if (!World) return;

	if (UR1GameInstance* GI = Cast<UR1GameInstance>(World->GetGameInstance()))
	{
		if (const FSkillDataRow* Data = GI->GetBossSkillData(SkillID))
		{
			CachedDamage = Data->Damage;
			CachedManaCost = Data->ManaCost;
			CachedCooldown = Data->Cooldown;
			CachedRange = Data->Range;

			UE_LOG(LogR1, Log, TEXT("[%s] SkillID '%s' resolved: Damage=%.1f Cooldown=%.1f Range=%.1f"),
				*GetName(), *SkillID.ToString(), CachedDamage, CachedCooldown, CachedRange);
		}
		else
		{
			// GameInstance는 SkillDataTable 포인터를 하나만 들고 있다. 보스/플레이어 테이블이
			// 따로 있으므로, 그 포인터가 다른 테이블을 가리키면 여기서 전부 실패한다.
			UE_LOG(LogR1, Error, TEXT("[%s] SkillID '%s' not found in the GameInstance BossSkillDataTable — damage and cooldown will be 0"),
				*GetName(), *SkillID.ToString());
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
	// 두 조건 중 하나라도 빠지면 쿨다운이 조용히 사라지므로 각각 이름을 찍어준다.
	if (CooldownGameplayEffectClass == nullptr)
	{
		UE_LOG(LogR1, Warning, TEXT("[%s] no Cooldown Gameplay Effect Class assigned — this skill has no cooldown"), *GetName());
	}
	else if (CachedCooldown <= 0.0f)
	{
		// 행을 못 찾았을 때도 여기로 온다 (캐시가 0으로 남으므로). 두 경우를 구분하려면
		// OnAvatarSet이 남긴 로그를 볼 것 — resolved 줄이 없으면 조회 자체가 실패한 것.
		UE_LOG(LogR1, Warning, TEXT("[%s] CachedCooldown is 0 for SkillID '%s' — either the row's Cooldown is 0 or the row was never found (see the OnAvatarSet log above)"),
			*GetName(), *SkillID.ToString());
	}

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
