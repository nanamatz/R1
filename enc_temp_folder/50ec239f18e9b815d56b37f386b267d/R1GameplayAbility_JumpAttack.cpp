#include "AbilitySystem/Abilities/R1GameplayAbility_JumpAttack.h"
#include "Character/R1Player.h"
#include "Character/R1Monster.h"
#include "Player/R1PlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToActorForce.h"

#include "Components/CapsuleComponent.h"
#include "R1GameplayTags.h"
#include "R1Define.h"
#include "System/R1GameInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "System/R1EquipmentManagerComponent.h"

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

	AR1Character* TargetMonster = PC->GetHighlightActor();
	if (TargetMonster == nullptr || TargetMonster->GetCreatureState() == ECreatureState::Dead)
	{
		return false;
	}	
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

	AR1Player* Player = Cast<AR1Player>(ActorInfo->AvatarActor.Get());

	if (Player == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AR1PlayerController* PC = Cast<AR1PlayerController>(Player->GetController());
	
	if(PC == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	CachedTarget = Cast<AR1Monster>(PC->GetHighlightActor());

	if(CachedTarget)
	{
		FVector Direction = CachedTarget->GetActorLocation() - Player->GetActorLocation();
		Direction.Z = 0.f;

		FRotator TargetRotation = Direction.Rotation();
		Player->SetActorRotation(TargetRotation);

		Player->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
		Player->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		Player->SetCreatureState(ECreatureState::Casting);
		PC->ResetMovementState();

		const float MyRadius = Player->GetCapsuleComponent()->GetScaledCapsuleRadius();
		const float TargetRadius = CachedTarget->GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector TargetOffset(MyRadius + TargetRadius + 10.f, 0.f, 0.f);

		if (MontageToPlay)
		{

			UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, MontageToPlay, 1.5f, NAME_None, false);

			MontageTask->OnCompleted.AddDynamic(this, &UR1GameplayAbility_JumpAttack::OnDashFinished);
			MontageTask->OnInterrupted.AddDynamic(this, &UR1GameplayAbility_JumpAttack::OnDashFinished);
			MontageTask->OnCancelled.AddDynamic(this, &UR1GameplayAbility_JumpAttack::OnDashFinished);

			MontageTask->ReadyForActivation();

			UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
				this,
				AttackEventTag, // 기다릴 태그
				nullptr,
				true,
				false
			);
			// 이벤트가 도착하면 -> OnAttackEventReceived 실행
			WaitEventTask->EventReceived.AddDynamic(this, &UR1GameplayAbility_JumpAttack::OnJumpAttackEventReceived);

			// Task 시작!
			WaitEventTask->ReadyForActivation();
		}
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}

		UAbilityTask_ApplyRootMotionMoveToActorForce* DashTask =
			UAbilityTask_ApplyRootMotionMoveToActorForce::ApplyRootMotionMoveToActorForce(
				this,
				TEXT("LeapQ_RootMotion"),
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

		//DashTask->OnFinished.AddDynamic(this, &UR1GameplayAbility_JumpAttack::OnDashFinished);

		DashTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

}

void UR1GameplayAbility_JumpAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AR1Player* Player = Cast<AR1Player>(ActorInfo->AvatarActor.Get()))
	{
		Player->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
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

void UR1GameplayAbility_JumpAttack::OnDashFinished()
{
	if (CurrentActorInfo == nullptr || CurrentActorInfo->AvatarActor.IsValid() == false)
	{
		return;
	}

	AR1Character* Attacker = Cast<AR1Character>(CurrentActorInfo->AvatarActor.Get());
	Attacker->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	if (Attacker)
	{
		if (Attacker->GetCreatureState() == ECreatureState::Dead)
		{
			Attacker->SetCreatureState(ECreatureState::Dead);
			UE_LOG(LogTemp, Warning, TEXT("Dead"));
		}
		else
		{
			Attacker->SetCreatureState(ECreatureState::Moving);
			UE_LOG(LogTemp, Warning, TEXT("Moving"));
		}
	}

	// 능력 정상 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UR1GameplayAbility_JumpAttack::OnDashInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UR1GameplayAbility_JumpAttack::OnJumpAttackEventReceived(FGameplayEventData Payload)
{
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	if (DamageEffect && SourceASC)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);
		EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Magnitude, CachedSkillDamage);

		if (EffectSpecHandle.IsValid() == false || EffectSpecHandle.Data.IsValid() == false)
		{
			return;
		}
		if (!CachedTarget || CachedTarget->GetCreatureState() == ECreatureState::Dead)
		{
			return;
		}
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CachedTarget);

		if (TargetASC)
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);
			FGameplayEventData PayloadData;
			FGameplayTag HitEventTag = R1GameplayTags::Ability_Attack;

			AActor* AvatarActor = GetAvatarActorFromActorInfo();
			PayloadData.Instigator = AvatarActor;
			PayloadData.Target = CachedTarget;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AvatarActor, HitEventTag, PayloadData);

			// [오디오 트리거] 장착된 무기에서 소리를 가져와 GameplayCue를 실행합니다.
			if (GameplayCueTag.IsValid() && AudioTag.IsValid())
			{
				if (AR1Player* Player = Cast<AR1Player>(AvatarActor))
				{
					if (UR1EquipmentManagerComponent* EquipManager = Player->GetEquipmentComponent())
					{
						SoundToPlay = EquipManager->GetSoundByTag(ER1EquipmentSlot::Weapon, AudioTag);
					}
				}

				if (SoundToPlay)
				{
					FGameplayCueParameters CueParams;
					CueParams.SourceObject = SoundToPlay;
					CueParams.Instigator = AvatarActor;
					CueParams.Location = CachedTarget->GetActorLocation();
					SourceASC->ExecuteGameplayCue(GameplayCueTag, CueParams);
				}
			}
		}

		/*AR1Character* Attacker = Cast<AR1Character>(CurrentActorInfo->AvatarActor.Get());
		if (Attacker)
		{
			if (Attacker->GetCreatureState() == ECreatureState::Dead)
			{
				Attacker->SetCreatureState(ECreatureState::Dead);
				UE_LOG(LogTemp, Warning, TEXT("Dead"));
			}
			else
			{
				Attacker->SetCreatureState(ECreatureState::Moving);
				UE_LOG(LogTemp, Warning, TEXT("Moving"));
			}
		}*/
	}
}

