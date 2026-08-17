#include "AbilitySystem/Abilities/R1GameplayAbility_BossCharge.h"
#include "R1LogChannels.h"
#include "R1GameplayTags.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Character/R1Player.h"

UR1GameplayAbility_BossCharge::UR1GameplayAbility_BossCharge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AttackEventTag = R1GameplayTags::Event_Montage_Attack;
}

void UR1GameplayAbility_BossCharge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 베이스는 텔레그래프를 '보스 위치 + 보스 Yaw'로 깐다.
	// 따라서 Super보다 먼저 타겟 쪽으로 돌아야 예고 레인과 실제 경로가 일치한다.
	ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (AvatarCharacter)
	{
		AActor* Target = nullptr;
		if (AAIController* AIC = Cast<AAIController>(AvatarCharacter->GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				Target = Cast<AActor>(BB->GetValueAsObject(BBKey_TargetActor));
			}
		}

		if (Target)
		{
			FVector ToTarget = Target->GetActorLocation() - AvatarCharacter->GetActorLocation();
			ToTarget.Z = 0.0f;

			if (!ToTarget.IsNearlyZero())
			{
				AvatarCharacter->SetActorRotation(ToTarget.Rotation());
			}
		}
		else
		{
			// 타겟이 없으면 현재 바라보는 방향으로 그대로 돌진한다 (중단하지는 않는다).
			UE_LOG(LogR1, Warning, TEXT("[BossCharge] no target on blackboard key '%s' — charging along current facing"), *BBKey_TargetActor.ToString());
		}

		// 이 방향이 곧 텔레그래프가 깔릴 방향이다. 돌진은 반드시 이 값을 쓴다.
		CachedChargeDirection = AvatarCharacter->GetActorForwardVector().GetSafeNormal2D();

		// 예고를 깐 뒤에도 계속 돌면 예고가 거짓말이 된다. 윈드업 동안 회전 정지.
		SetRotationLocked(true);
	}

	HitActors.Reset();

	// 텔레그래프 + 몽타주 + Event.Montage.Attack 대기는 베이스가 처리한다.
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UR1GameplayAbility_BossCharge::OnAttackEventReceived(FGameplayEventData Payload)
{
	ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarCharacter)
	{
		return;
	}

	const FVector StartLocation = AvatarCharacter->GetActorLocation();

	// 텔레그래프를 깔던 시점의 방향을 그대로 쓴다. 여기서 forward를 다시 읽으면
	// 윈드업 중 회전한 만큼 예고와 경로가 갈라져 회피가 불가능해진다.
	const FVector ChargeDirection = CachedChargeDirection.IsNearlyZero()
		? AvatarCharacter->GetActorForwardVector().GetSafeNormal2D()
		: CachedChargeDirection;

	// 메시가 다른 곳을 보며 옆으로 미끄러지지 않도록 실제 회전도 그 방향으로 맞춘다.
	AvatarCharacter->SetActorRotation(ChargeDirection.Rotation());

	const FVector TargetLocation = StartLocation + ChargeDirection * ChargeDistance;

	// 돌진 중에는 플레이어를 밀지 않고 통과한다 (JumpAttack이 쓰는 방식과 동일).
	SetPawnCollisionPassThrough(true);

	LastSweepLocation = StartLocation;

	UAbilityTask_ApplyRootMotionMoveToForce* ChargeTask =
		UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
			this,
			TEXT("BossCharge_RootMotion"),
			TargetLocation,
			ChargeDuration,
			/*bSetNewMovementMode=*/false,
			MOVE_None,
			/*bRestrictSpeedToExpected=*/false,
			/*PathOffsetCurve=*/nullptr,
			ERootMotionFinishVelocityMode::ClampVelocity,
			FVector::ZeroVector,
			0.0f
		);

	if (!ChargeTask)
	{
		SetPawnCollisionPassThrough(false);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 이 태스크는 항상 Duration 만큼 돌고 끝난다. 목적지 도달 여부에 따라 델리게이트가
	// 갈리므로 둘 다 같은 핸들러에 묶는다.
	ChargeTask->OnTimedOut.AddDynamic(this, &UR1GameplayAbility_BossCharge::OnChargeFinished);
	ChargeTask->OnTimedOutAndDestinationReached.AddDynamic(this, &UR1GameplayAbility_BossCharge::OnChargeFinished);
	ChargeTask->ReadyForActivation();

	// 돌진하는 내내 지나간 구간을 스윕한다. 시작 시점에 한 번에 판정하면
	// 돌진 도중 피하는 플레이어도 맞아버린다.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SweepTimerHandle, this, &UR1GameplayAbility_BossCharge::SweepAndDamage, SweepInterval, true);
	}
}

void UR1GameplayAbility_BossCharge::SweepAndDamage()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UWorld* World = GetWorld();

	if (!AvatarActor || !SourceASC || !World || !DamageEffect)
	{
		return;
	}

	const FVector CurrentLocation = AvatarActor->GetActorLocation();

	// 직전 스윕 위치 → 현재 위치 구간만 검사. 프레임 사이를 건너뛰지 않게 한다.
	TArray<FHitResult> Hits;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	World->SweepMultiByObjectType(
		Hits,
		LastSweepLocation,
		CurrentLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(ChargeHalfWidth),
		QueryParams);

	LastSweepLocation = CurrentLocation;

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();

		// 플레이어만, 그리고 이번 돌진에서 아직 안 맞은 대상만.
		if (!HitActor || !HitActor->IsA<AR1Player>() || HitActors.Contains(HitActor))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC)
		{
			continue;
		}

		HitActors.Add(HitActor);

		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);
		if (!SpecHandle.IsValid())
		{
			continue;
		}
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Magnitude, CachedDamage);
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

		FGameplayCueParameters CueParams;
		CueParams.SourceObject = GetHitImpactEffect();
		CueParams.Instigator = AvatarActor;
		CueParams.Location = HitActor->GetActorLocation() + FVector(0, 0, 50.0f);
		CueParams.Normal = (CurrentLocation - HitActor->GetActorLocation()).GetSafeNormal();
		SourceASC->ExecuteGameplayCue(R1GameplayTags::GameplayCue_Weapon_Impact, CueParams);

		UE_LOG(LogR1, Log, TEXT("BossCharge: hit %s"), *HitActor->GetName());
	}
}

void UR1GameplayAbility_BossCharge::OnChargeFinished()
{
	// 두 델리게이트가 모두 걸려 있으므로 중복 진입을 막는다.
	if (!IsActive())
	{
		return;
	}

	// 마지막 구간을 놓치지 않도록 한 번 더 스윕한 뒤 정리한다.
	SweepAndDamage();
	StopChargeSweep();
	SetPawnCollisionPassThrough(false);

	UE_LOG(LogR1, Log, TEXT("BossCharge: finished (targets hit: %d)"), HitActors.Num());

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UR1GameplayAbility_BossCharge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 취소(페이즈 전환의 CancelAbilities 등)로 끝나도 충돌·타이머·회전은 반드시 원복한다.
	StopChargeSweep();
	SetPawnCollisionPassThrough(false);
	SetRotationLocked(false);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UR1GameplayAbility_BossCharge::StopChargeSweep()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SweepTimerHandle);
	}
}

void UR1GameplayAbility_BossCharge::SetRotationLocked(bool bLocked)
{
	ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UCharacterMovementComponent* MoveComp = AvatarCharacter ? AvatarCharacter->GetCharacterMovement() : nullptr;
	if (!MoveComp)
	{
		return;
	}

	if (bLocked)
	{
		if (bRotationLocked)
		{
			return;
		}
		bCachedUseControllerDesiredRotation = MoveComp->bUseControllerDesiredRotation;
		bCachedOrientRotationToMovement = MoveComp->bOrientRotationToMovement;

		MoveComp->bUseControllerDesiredRotation = false;
		MoveComp->bOrientRotationToMovement = false;
		bRotationLocked = true;
	}
	else if (bRotationLocked)
	{
		MoveComp->bUseControllerDesiredRotation = bCachedUseControllerDesiredRotation;
		MoveComp->bOrientRotationToMovement = bCachedOrientRotationToMovement;
		bRotationLocked = false;
	}
}

void UR1GameplayAbility_BossCharge::SetPawnCollisionPassThrough(bool bPassThrough)
{
	ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UCapsuleComponent* Capsule = AvatarCharacter ? AvatarCharacter->GetCapsuleComponent() : nullptr;
	if (!Capsule)
	{
		return;
	}

	if (bPassThrough)
	{
		if (bCollisionModified)
		{
			return;
		}
		CachedPawnResponse = Capsule->GetCollisionResponseToChannel(ECC_Pawn);
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		bCollisionModified = true;
	}
	else if (bCollisionModified)
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, CachedPawnResponse);
		bCollisionModified = false;
	}
}
