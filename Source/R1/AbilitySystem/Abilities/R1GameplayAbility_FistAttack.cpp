


#include "AbilitySystem/Abilities/R1GameplayAbility_FistAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h" // 몽타주 Task
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"  // 이벤트 대기 Task
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "Player/R1PlayerController.h"
#include "Character/R1Player.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/R1Character.h"
#include "R1GameplayTags.h"

UR1GameplayAbility_FistAttack::UR1GameplayAbility_FistAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UR1GameplayAbility_FistAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}
	return true;
}

void UR1GameplayAbility_FistAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AR1Character* Attacker = Cast<AR1Character>(ActorInfo->AvatarActor);

	if (Attacker && FistMontage)
	{
		Attacker->SetCreatureState(ECreatureState::Casting);
		float AttackRate = 1.0f; // 기본 속도

		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			bool bFound = false;
			// GAS에서 현재 AttackSpeed 스탯 값을 읽어옵니다.
			float CurrentAttackSpeed = ASC->GetGameplayAttributeValue(UR1AttributeSet::GetAttackSpeedAttribute(), bFound);

			if (bFound)
			{
				AttackRate = CurrentAttackSpeed;
			}
		}
		FName StartSectionName = (ComboIndex == 0) ? FName("Combo1") : FName("Combo2");

		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			FistMontage, // 재생할 몽타주
			AttackRate,
			StartSectionName,
			false
		);

		MontageTask->OnCompleted.AddDynamic(this, &UR1GameplayAbility_FistAttack::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UR1GameplayAbility_FistAttack::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UR1GameplayAbility_FistAttack::OnMontageEnded);

		MontageTask->ReadyForActivation();

		ComboIndex = (ComboIndex + 1) % 2;

		UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			AttackEventTag, // 기다릴 태그
			nullptr,
			true,
			false
		);
		// 이벤트가 도착하면 -> OnAttackEventReceived 실행
		WaitEventTask->EventReceived.AddDynamic(this, &UR1GameplayAbility_FistAttack::OnAttackEventReceived);

		// Task 시작!
		WaitEventTask->ReadyForActivation();

	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UR1GameplayAbility_FistAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UR1GameplayAbility_FistAttack::OnMontageEnded()
{
	// 애니메이션이 다 끝났으니 어빌리티를 완전히 종료합니다.
	if (CurrentActorInfo == nullptr || CurrentActorInfo->AvatarActor.IsValid() == false)
	{
		return;
	}

	AR1Character* Attacker = Cast<AR1Character>(CurrentActorInfo->AvatarActor.Get());
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
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}	

void UR1GameplayAbility_FistAttack::OnAttackEventReceived(FGameplayEventData Payload)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	AR1Character* SourceCharacter = Cast<AR1Character>(AvatarActor);

	if (!SourceCharacter) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	AActor* TargetActor = nullptr;

	if (DamageEffect && SourceASC)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);
		if (EffectSpecHandle.IsValid() == false || EffectSpecHandle.Data.IsValid() == false)
		{
			return;
		}
		if (AR1PlayerController* PC = Cast<AR1PlayerController>(SourceCharacter->GetController()))
		{
			if (PC->TargetAttackActor)
			{
				TargetActor = PC->TargetAttackActor;

				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

				if (TargetASC)
				{
					SourceASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);

					FGameplayEventData PayloadData;
					PayloadData.Instigator = SourceCharacter;
					PayloadData.Target = TargetActor;

					FGameplayTag HitEventTag = R1GameplayTags::Ability_FistAttack;

					// 나 자신에게 이벤트를 보내서, 내 몸에 장착된 패시브 GA들이 듣고 반응하게 합니다.
					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(SourceCharacter, HitEventTag, PayloadData);

					if (GameplayCueTag.IsValid() && AudioTag.IsValid())
					{
						USoundBase* SoundToPlay = nullptr;

						if (SoundToPlay)
						{
							FGameplayCueParameters CueParams;
							CueParams.SourceObject = SoundToPlay;
							CueParams.Instigator = SourceCharacter;

							FVector StartLoc = SourceCharacter->GetActorLocation() + FVector(0, 0, 50.0f); // 명치를 향하도록 Z축 보정
							FVector EndLoc = TargetActor->GetActorLocation() + FVector(0, 0, 50.0f);

							FHitResult HitResult;
							FCollisionQueryParams TraceParams;
							TraceParams.AddIgnoredActor(SourceCharacter);

							// 공격자의 명치에서 타겟의 명치로 보이지 않는 선을 긋습니다.
							bool bHit = SourceCharacter->GetWorld()->LineTraceSingleByChannel(
								HitResult, StartLoc, EndLoc, ECC_Visibility, TraceParams);

							if (bHit)
							{
								// 캡슐(피부)에 맞았다면 그 정확한 표면 지점과 각도를 사용합니다.
								CueParams.Location = HitResult.ImpactPoint;
								CueParams.Normal = HitResult.ImpactNormal;
							}
							else
							{
								// 만약 장애물 등으로 빗나갔다면(예외 상황) 기본 위치로 세팅
								CueParams.Location = TargetActor->GetActorLocation() + FVector(0, 0, 50.0f);
								CueParams.Normal = (StartLoc - EndLoc).GetSafeNormal();
							}

							SourceASC->ExecuteGameplayCue(GameplayCueTag, CueParams);
						}
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Player Attacked, But TargetActor is NULL!"));
				}
			}
		}
	}
}
