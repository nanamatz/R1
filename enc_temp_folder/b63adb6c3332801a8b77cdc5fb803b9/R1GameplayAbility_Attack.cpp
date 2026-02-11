

#include "AbilitySystem/Abilities/R1GameplayAbility_Attack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h" // 몽타주 Task
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"  // 이벤트 대기 Task
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "Player/R1PlayerController.h"
#include "Character/R1Player.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/R1Character.h"

UR1GameplayAbility_Attack::UR1GameplayAbility_Attack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)	
{
	if (DamageEffect)
	{

	}
}

bool UR1GameplayAbility_Attack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false) 
	{
		return false;
	}
	return true;
}

void UR1GameplayAbility_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AR1Character* Attacker = Cast<AR1Character>(ActorInfo->AvatarActor);

	// 1. 자원(마나/스태미나) 소모 및 쿨타임 확인
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 2. 몽타주 재생 (PlayMontageAndWait Task 사용)
	// 이 Task는 애니메이션이 끝날 때까지 어빌리티가 끝나지 않게 잡아주는 역할도 합니다.


	if (Attacker && Attacker->AttackMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			Attacker->AttackMontage, // 재생할 몽타주
			1.0f,
			NAME_None,
			false
		);

		MontageTask->OnCompleted.AddDynamic(this, &UR1GameplayAbility_Attack::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UR1GameplayAbility_Attack::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UR1GameplayAbility_Attack::OnMontageEnded);

		MontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			AttackEventTag, // 기다릴 태그
			nullptr,
			false,
			false
		);

		// 이벤트가 도착하면 -> OnHitEventReceived 실행
		WaitEventTask->EventReceived.AddDynamic(this, &UR1GameplayAbility_Attack::OnAttackEventReceived);

		// Task 시작!
		WaitEventTask->ReadyForActivation();


		/*Attacker->PlayAnimMontage(Attacker->AttackMontage);*/
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}

}

void UR1GameplayAbility_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UR1GameplayAbility_Attack::OnMontageEnded()
{
	// 애니메이션이 다 끝났으니 어빌리티를 완전히 종료합니다.
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UR1GameplayAbility_Attack::OnAttackEventReceived(FGameplayEventData Payload)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	AR1Character* SourceCharacter = Cast<AR1Character>(AvatarActor);

	if (!SourceCharacter) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	float AttackRange = 0.f;
	float AttackRadius = 0.f;
	AActor* TargetActor = nullptr;

	if (DamageEffect && SourceASC)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);

		if (SourceCharacter->IsPlayerControlled())
		{
			if (AR1PlayerController* PC = Cast<AR1PlayerController>(SourceCharacter->GetController()))
			{
				if (PC->TargetAttackActor)
				{
					TargetActor = PC->TargetAttackActor;

					UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

					if (TargetASC)
					{
						SourceASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Player Attacked, But TargetActor is NULL!"));

					// (선택사항) 논타겟팅 보정: 타겟이 없으면 몬스터처럼 앞쪽 Sweep을 시도할 수도 있습니다.
				}
			}
		}
		else
		{
			AttackRange = SourceASC->GetNumericAttribute(UR1AttributeSet::GetAttackRangeAttribute());
			AttackRadius = SourceASC->GetNumericAttribute(UR1AttributeSet::GetAttackRadiusAttribute());

			TArray<FHitResult> HitResults;
			FVector Start = SourceCharacter->GetActorLocation();

			// [변경] 하드코딩된 변수 대신, Attribute에서 가져온 값을 사용!
			FVector End = Start + (SourceCharacter->GetActorForwardVector() * AttackRange);

			FCollisionQueryParams Params;
			Params.AddIgnoredActor(SourceCharacter);

			bool bHit = GetWorld()->SweepMultiByChannel(
				HitResults,
				Start,
				End,
				FQuat::Identity,
				ECC_GameTraceChannel1,
				FCollisionShape::MakeSphere(AttackRadius), // [변경] 여기도 Attribute 값 사용
				Params
			);

			DrawDebugSphere(GetWorld(), Start, AttackRadius, 16, FColor::Green, false, 1.f);
			DrawDebugSphere(GetWorld(), End, AttackRadius, 16, FColor::Blue, false, 1.f);

			for (const FHitResult& HitResult : HitResults)
			{
				AR1Player* HitPlayer = Cast<AR1Player>(HitResult.GetActor());

				if (HitPlayer)
				{
					UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitPlayer);
					SourceASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);
				}
			}
		}
	}
}
