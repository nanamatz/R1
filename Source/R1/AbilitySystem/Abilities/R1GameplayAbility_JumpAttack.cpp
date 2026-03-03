#include "AbilitySystem/Abilities/R1GameplayAbility_JumpAttack.h"
#include "Character/R1Player.h"
#include "Character/R1Monster.h"
#include "Player/R1PlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToActorForce.h"
#include "Components/CapsuleComponent.h"
#include "R1GameplayTags.h"
#include "R1Define.h"
#include "System/R1GameInstance.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"

UR1GameplayAbility_JumpAttack::UR1GameplayAbility_JumpAttack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UR1GameplayAbility_JumpAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}

	AR1Player* PlayerCharacter = Cast<AR1Player>(ActorInfo->AvatarActor.Get());
	if (!PlayerCharacter) return false;

	AR1PlayerController* PC = Cast<AR1PlayerController>(PlayerCharacter->GetController());
	if (!PC) return false;

	AR1Monster* TargetMonster = Cast<AR1Monster>(PC->GetHighlightActor());
	if (TargetMonster == nullptr || TargetMonster->GetCreatureState() == ECreatureState::Dead) return false;

	float DistanceToTarget = FVector::Distance(PlayerCharacter->GetActorLocation(), TargetMonster->GetActorLocation());
	if (DistanceToTarget > CachedSkillRange)
	{
		return false;
	}

	return true;
}

void UR1GameplayAbility_JumpAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 1. 자원(마나/스태미나) 소모 및 쿨타임 확인
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (JumpMontage != nullptr)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, JumpMontage);
		PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnDashInterrupted);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnDashInterrupted);
		PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnDashInterrupted);
		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		// 몽타주를 빼먹었을 경우 개발자가 알기 쉽게 빨간색 에러를 띄우고 스킬을 즉시 종료합니다.
		UE_LOG(LogTemp, Error, TEXT("🚨 [GA_JumpAttack] 몽타주 에셋이 할당되지 않았습니다! 블루프린트를 확인하세요."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}

	AR1Player* Player = Cast<AR1Player>(ActorInfo->AvatarActor.Get());
	AR1PlayerController* PC = Cast<AR1PlayerController>(Player->GetController());

	CachedTarget = Cast<AR1Monster>(PC->GetHighlightActor());

	if(CachedTarget == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("nullptr!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 벽 넘기 (비행 중 지형 충돌 무시)
	Player->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	Player->SetCreatureState(ECreatureState::Casting);

	// 1. 애니메이션 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, JumpMontage, 1.0f, NAME_None, false);

	MontageTask->ReadyForActivation();

	// 2. 도약 (Root Motion)
	if (CachedTarget && CachedTarget->IsActorBeingDestroyed() == false)
	{
		UAbilityTask_ApplyRootMotionMoveToActorForce* DashTask = 
			UAbilityTask_ApplyRootMotionMoveToActorForce::ApplyRootMotionMoveToActorForce(
				this,
				NAME_None,
				CachedTarget,
				CachedTarget->GetActorLocation(),
				ERootMotionMoveToActorTargetOffsetType::AlignFromTargetToSource,
				DashDuration,
				nullptr,
				nullptr,
				false,
				MOVE_Walking,
				false,
				JumpHeightCurve,
				nullptr,
				ERootMotionFinishVelocityMode::SetVelocity,
				FVector::ZeroVector,
				0.0f,
				false
			);

		DashTask->OnFinished.AddDynamic(this, &UR1GameplayAbility_JumpAttack::OnDashFinished);
		DashTask->ReadyForActivation();
	}
}

void UR1GameplayAbility_JumpAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AR1Player* Player = Cast<AR1Player>(ActorInfo->AvatarActor.Get()))
	{
		Player->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

		// 2. 캐릭터 상태 원상 복구 (죽은 게 아니라면 Idle로)
		if (Player->GetCreatureState() != ECreatureState::Dead)
		{
			Player->SetCreatureState(ECreatureState::Idle);
		}
	}

	CachedTarget = nullptr; // 안전을 위해 포인터 비우기

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UR1GameplayAbility_JumpAttack::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnAvatarSet: ActorInfo 또는 AvatarActor가 유효하지 않습니다!"));
		return;
	}

	UWorld* World = ActorInfo->AvatarActor->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnAvatarSet: AvatarActor에서 World를 가져올 수 없습니다!"));
		return;
	}

	if (UR1GameInstance* GameInstance = Cast<UR1GameInstance>(World->GetGameInstance()))
	{
		if (const FSkillDataRow* SkillData = GameInstance->GetSkillData(SkillID))
		{
			CachedSkillDamage = SkillData->Damage;
			CachedSkillRange = SkillData->Range;
			CachedManaCost = SkillData->ManaCost;

			UE_LOG(LogTemp, Warning, TEXT("[OnAvatarSet] 스킬 데이터 로드 완료! 데미지: %f"), CachedSkillDamage);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[OnAvatarSet] SkillID '%s'에 해당하는 데이터를 찾을 수 없습니다!"), *SkillID.ToString());
		}
	}
}

bool UR1GameplayAbility_JumpAttack::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	//Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC)
	{
		// 플레이어의 현재 마나를 가져옵니다. 
		float CurrentMana = ASC->GetNumericAttribute(UPlayerAttributeSet::GetManaAttribute());

		// 현재 마나가 데이터 테이블에서 가져온 마나 소모량보다 적으면 스킬 시전 불가!
		if (CurrentMana < CachedManaCost)
		{
			UE_LOG(LogTemp, Warning, TEXT("마나가 부족합니다! 필요 마나: %f, 현재 마나: %f"), CachedManaCost, CurrentMana);
			return false;
		}
	}
	return true;
}

void UR1GameplayAbility_JumpAttack::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	//Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
	if (CostGameplayEffectClass)
	{
		// 마나를 깎을 GE 명세서를 만듭니다.
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CostGameplayEffectClass, GetAbilityLevel(Handle, ActorInfo));

		if (SpecHandle.IsValid())
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Cost, -CachedManaCost);

			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}
}

void UR1GameplayAbility_JumpAttack::OnDashFinished(bool bDestinationReached, bool bTimedOut, FVector FinalTargetLocation)
{
	if (CachedTarget && DamageEffect && CachedTarget->GetCreatureState() != ECreatureState::Dead)
	{
		UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CachedTarget);

		if (SourceASC && TargetASC)
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
			Context.AddSourceObject(this);

			FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, Context);
			EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Magnitude, CachedSkillDamage);

			SourceASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);


			FGameplayEventData PayloadData;
			FGameplayTag HitEventTag = R1GameplayTags::Ability_Attack;

			AActor* AvatarActor = GetAvatarActorFromActorInfo();
			PayloadData.Instigator = AvatarActor;
			PayloadData.Target = CachedTarget;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AvatarActor, HitEventTag, PayloadData);
		}
	}

	// 능력 정상 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UR1GameplayAbility_JumpAttack::OnDashInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

