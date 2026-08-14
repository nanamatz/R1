#include "AbilitySystem/Abilities/R1GameplayAbility_BossLeap.h"
#include "R1LogChannels.h"
#include "R1GameplayTags.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToActorForce.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Character/R1Player.h"
#include "Data/R1TelegraphData.h"
#include "Object/R1TelegraphActor.h"

UR1GameplayAbility_BossLeap::UR1GameplayAbility_BossLeap()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AttackEventTag = R1GameplayTags::Event_Montage_Attack;
}

void UR1GameplayAbility_BossLeap::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 베이스(BossAttackBase)의 ActivateAbility는 텔레그래프를 '보스 위치'에 깔고 몽타주 이벤트를 기다린다.
	// 도약은 착지 지점에 깔고 루트모션 종료로 판정해야 하므로 베이스를 호출하지 않고 직접 구성한다.

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 블랙보드에서 타겟 획득
	CachedTarget = nullptr;
	if (AAIController* AIC = Cast<AAIController>(AvatarCharacter->GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			CachedTarget = Cast<AActor>(BB->GetValueAsObject(BBKey_TargetActor));
		}
	}

	if (!CachedTarget)
	{
		UE_LOG(LogR1, Warning, TEXT("[BossLeap] blackboard key '%s' has no target actor"), *BBKey_TargetActor.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. 텔레그래프를 '타겟의 현재 위치'에 배치.
	//    도약은 타겟을 추적하므로 착지점과 완전히 일치하지는 않는다 (허용된 오차).
	if (TelegraphData && TelegraphActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = AvatarCharacter;
		SpawnParams.Instigator = AvatarCharacter;

		const FRotator SpawnRotation = FRotator(0.0f, AvatarCharacter->GetActorRotation().Yaw, 0.0f);
		AR1TelegraphActor* TelegraphActor = GetWorld()->SpawnActor<AR1TelegraphActor>(
			TelegraphActorClass, CachedTarget->GetActorLocation(), SpawnRotation, SpawnParams);

		if (TelegraphActor)
		{
			TelegraphActor->InitializeTelegraph(TelegraphData);
		}
	}

	// 3. 몽타주 (착지 판정은 몽타주가 아니라 루트모션 종료에 맞춘다)
	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("BossLeapMontage"), MontageToPlay);

		if (MontageTask)
		{
			MontageTask->OnInterrupted.AddDynamic(this, &UR1GameplayAbility_BossLeap::OnMontageEnded);
			MontageTask->OnCancelled.AddDynamic(this, &UR1GameplayAbility_BossLeap::OnMontageEnded);
			MontageTask->ReadyForActivation();
		}

		// 몽타주가 즉시 실패하면 위 호출 안에서 EndAbility까지 동기 실행될 수 있다.
		// 종료된 인스턴스에 태스크를 더 붙이면 ensure가 발생하므로 중단한다.
		if (!IsActive())
		{
			return;
		}
	}

	// 4. 루트모션 도약. 타겟 액터를 직접 추적하므로 움직이는 플레이어를 따라간다.
	const float MyRadius = AvatarCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector TargetOffset(MyRadius + 60.f, 0.f, 0.f);

	UAbilityTask_ApplyRootMotionMoveToActorForce* LeapTask =
		UAbilityTask_ApplyRootMotionMoveToActorForce::ApplyRootMotionMoveToActorForce(
			this,
			TEXT("BossLeap_RootMotion"),
			CachedTarget,
			TargetOffset,
			ERootMotionMoveToActorTargetOffsetType::AlignFromTargetToSource,
			DashDuration,
			nullptr,
			nullptr,
			true,
			MOVE_Flying,
			true,
			JumpHeightCurve,
			nullptr,
			ERootMotionFinishVelocityMode::SetVelocity,
			FVector::ZeroVector,
			0.0f,
			false
		);

	if (!LeapTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	LeapTask->OnFinished.AddDynamic(this, &UR1GameplayAbility_BossLeap::OnLeapFinished);
	LeapTask->ReadyForActivation();
}

void UR1GameplayAbility_BossLeap::OnLeapFinished(bool bReachedDestination, bool bTimedOut, FVector FinalTargetLocation)
{
	// 도착이든 타임아웃이든 착지 처리는 동일하게 수행한다.
	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		ApplyLandingDamage(AvatarActor->GetActorLocation());
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UR1GameplayAbility_BossLeap::ApplyLandingDamage(const FVector& LandingLocation)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	if (!AvatarActor || !SourceASC || !TelegraphData || !DamageEffect)
	{
		return;
	}

	const float Radius = TelegraphData->TelegraphSize.X;

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);
	if (!EffectSpecHandle.IsValid())
	{
		return;
	}
	EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Magnitude, CachedDamage);

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(AvatarActor);

	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), LandingLocation, Radius, ObjectTypes, AR1Player::StaticClass(), IgnoreActors, OverlappedActors);

	for (AActor* OverlappedActor : OverlappedActors)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlappedActor);
		if (TargetASC)
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);

			// [VFX] 피해를 준 플레이어 위치에 무기 임팩트 큐 실행
			FGameplayCueParameters CueParams;
			CueParams.SourceObject = GetHitImpactEffect();
			CueParams.Instigator = AvatarActor;
			CueParams.Location = OverlappedActor->GetActorLocation() + FVector(0, 0, 50.0f);
			CueParams.Normal = (LandingLocation - OverlappedActor->GetActorLocation()).GetSafeNormal();
			SourceASC->ExecuteGameplayCue(R1GameplayTags::GameplayCue_Weapon_Impact, CueParams);
		}
	}

	UE_LOG(LogR1, Log, TEXT("BossLeap: landed, hit %d actors"), OverlappedActors.Num());
}
