#include "AbilitySystem/Abilities/R1GameplayAbility_BladeWave.h"
#include "R1LogChannels.h"
#include "R1GameplayTags.h"
#include "Character/R1Player.h"
#include "Player/R1PlayerController.h"
#include "System/R1GameInstance.h"
#include "System/R1EquipmentManagerComponent.h"
#include "Object/R1BladeWaveProjectile.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UR1GameplayAbility_BladeWave::UR1GameplayAbility_BladeWave(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	SkillType = ER1SkillType::Active;

	// 차지 중 피격 리액션 차단 (HitReact의 ActivationBlockedTags와 짝)
	ActivationOwnedTags.AddTag(R1GameplayTags::Character_State_UnInterruptable);
}

void UR1GameplayAbility_BladeWave::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AR1Player* Player = Cast<AR1Player>(ActorInfo->AvatarActor.Get());
	if (Player == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 어느 슬롯(Q/W/E/R)에 꽂혀 있는지 확인해 그 키의 릴리즈 이벤트만 기다린다.
	ER1SkillSlot MySlot = ER1SkillSlot::None;
	if (UR1EquipmentManagerComponent* EquipComp = Player->GetEquipmentComponent())
	{
		MySlot = EquipComp->GetSlotForHandle(Handle);
	}
	if (MySlot == ER1SkillSlot::None)
	{
		UE_LOG(LogR1, Warning, TEXT("[BladeWave] 스킬 슬롯을 찾지 못해 시전을 취소합니다."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 차지 몽타주 재생(Charge → ChargeLoop 루프) + 발사 노티파이 대기
	if (PlayAttackMontageAndWaitForEvent(Player, AttackEventTag) == false)
	{
		// 몽타주 동기 실패로 헬퍼 안에서 이미 EndAbility된 경우는 이중 종료를 피한다.
		if (IsActive())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
		return;
	}

	bReleased = false;
	ChargeStartTime = GetWorld()->GetTimeSeconds();

	UAbilityTask_WaitGameplayEvent* WaitReleaseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		R1GameplayTags::GetSkillReleaseTag(MySlot),
		nullptr,
		true,	// OnlyTriggerOnce
		true	// OnlyMatchExact
	);
	WaitReleaseTask->EventReceived.AddDynamic(this, &UR1GameplayAbility_BladeWave::OnSkillKeyReleased);
	WaitReleaseTask->ReadyForActivation();
}

void UR1GameplayAbility_BladeWave::OnSkillKeyReleased(FGameplayEventData Payload)
{
	if (bReleased)
	{
		return;
	}

	const float Elapsed = GetWorld()->GetTimeSeconds() - ChargeStartTime;

	// 최소 차지 미달 → 무료 취소 (마나/쿨다운 없음)
	if (Elapsed < MinChargeTime)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	bReleased = true;

	const float Denominator = FMath::Max(MaxChargeTime - MinChargeTime, KINDA_SMALL_NUMBER);
	const float ChargeRatio = FMath::Clamp((Elapsed - MinChargeTime) / Denominator, 0.0f, 1.0f);
	CachedDamageScale = FMath::Lerp(MinDamageScale, MaxDamageScale, ChargeRatio);
	CachedSizeScale = FMath::Lerp(MinSizeScale, MaxSizeScale, ChargeRatio);

	AR1Player* Player = Cast<AR1Player>(GetAvatarActorFromActorInfo());
	if (Player == nullptr)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 릴리즈 순간의 커서 위치로 캐릭터를 회전시키고 발사 방향을 확정한다.
	CachedFireDirection = Player->GetActorForwardVector();
	if (AR1PlayerController* PC = Cast<AR1PlayerController>(Player->GetController()))
	{
		FHitResult CursorHit;
		if (PC->GetHitResultUnderCursor(ECC_GameTraceChannel2, false, CursorHit))
		{
			FVector Direction = CursorHit.Location - Player->GetActorLocation();
			Direction.Z = 0.0f;
			if (Direction.Normalize())
			{
				CachedFireDirection = Direction;
				Player->SetActorRotation(Direction.Rotation());
			}
		}
	}
	CachedFireDirection.Z = 0.0f;
	CachedFireDirection = CachedFireDirection.GetSafeNormal();

	// 발사 섹션으로 점프 → 섹션 내 노티파이가 OnAttackEventReceived를 호출한다.
	MontageJumpToSection(FireSectionName);
}

void UR1GameplayAbility_BladeWave::OnAttackEventReceived(FGameplayEventData Payload)
{
	// 발사 확정 전(차지 중) 노티파이 수신은 무시 — 몽타주 구성 실수 안전망
	if (bReleased == false)
	{
		UE_LOG(LogR1, Warning, TEXT("[BladeWave] 발사 노티파이가 릴리즈 전에 도착 — 몽타주 섹션 연결(ChargeLoop 루프)을 확인하세요."));
		return;
	}

	// 이 시점에 마나 차감 + 쿨다운 시작 (릴리즈 전 취소는 비용 없음)
	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo) == false)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	AR1Player* Player = Cast<AR1Player>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (Player == nullptr || SourceASC == nullptr || ProjectileClass == nullptr || DamageEffect == nullptr)
	{
		UE_LOG(LogR1, Error, TEXT("[BladeWave] 발사 실패 — ProjectileClass/DamageEffect 설정을 확인하세요."));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const FVector SpawnLocation = Player->GetActorLocation() + CachedFireDirection * 100.0f;
	const FRotator SpawnRotation = CachedFireDirection.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Player;
	SpawnParams.Instigator = Player;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AR1BladeWaveProjectile* Projectile = GetWorld()->SpawnActor<AR1BladeWaveProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (Projectile)
	{
		Projectile->ProjectileMovement->MaxSpeed = ProjectileSpeed;
		Projectile->ProjectileMovement->Velocity = CachedFireDirection * ProjectileSpeed;
		Projectile->SetChargeScale(CachedSizeScale);

		if (CachedRange > 0.0f && ProjectileSpeed > 0.0f)
		{
			Projectile->SetLifeSpan(CachedRange / ProjectileSpeed);
		}

		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Magnitude, CachedSkillDamage * CachedDamageScale);
			Projectile->DamageSpecHandle = SpecHandle;
		}
	}

	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, Player->GetActorLocation());
	}
}

void UR1GameplayAbility_BladeWave::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UR1GameplayAbility_BladeWave::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 취소 종료 시 차지 루프 몽타주를 직접 정지해야 한다.
	// (PlayAttackMontageAndWaitForEvent가 bStopWhenAbilityEnds=false로 재생하므로 자동으로 멈추지 않음)
	if (bWasCancelled)
	{
		MontageStop();
	}

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (AR1Player* Player = Cast<AR1Player>(ActorInfo->AvatarActor.Get()))
		{
			if (Player->GetCreatureState() != ECreatureState::Dead)
			{
				Player->SetCreatureState(ECreatureState::Idle);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UR1GameplayAbility_BladeWave::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid())
	{
		return;
	}

	UWorld* World = ActorInfo->AvatarActor->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (UR1GameInstance* GameInstance = Cast<UR1GameInstance>(World->GetGameInstance()))
	{
		if (const FSkillDataRow* SkillData = GameInstance->GetSkillData(SkillID))
		{
			CachedSkillDamage = SkillData->Damage;
			CachedManaCost = SkillData->ManaCost;
			CachedCooldown = SkillData->Cooldown;
			CachedRange = SkillData->Range;
		}
		else
		{
			UE_LOG(LogR1, Error, TEXT("[BladeWave] SkillID '%s'에 해당하는 스킬 데이터를 찾을 수 없습니다!"), *SkillID.ToString());
		}
	}
}

bool UR1GameplayAbility_BladeWave::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (CachedManaCost <= 0.0f)
	{
		return true;
	}

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		const float CurrentMana = ASC->GetNumericAttribute(UPlayerAttributeSet::GetManaAttribute());
		if (CurrentMana < CachedManaCost)
		{
			UE_LOG(LogR1, Warning, TEXT("[BladeWave] 마나 부족 — 필요: %f, 현재: %f"), CachedManaCost, CurrentMana);
			return false;
		}
	}
	return true;
}

void UR1GameplayAbility_BladeWave::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (CostGameplayEffectClass && CachedManaCost > 0.0f)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CostGameplayEffectClass, GetAbilityLevel(Handle, ActorInfo));
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Cost, -CachedManaCost);
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}
}

void UR1GameplayAbility_BladeWave::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	// 쿨다운 GE의 지속시간을 DT_SkillData의 Cooldown 값으로 주입한다. (Super 미호출 — GE 고정 duration 대체)
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
