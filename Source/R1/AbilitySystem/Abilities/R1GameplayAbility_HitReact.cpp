


#include "AbilitySystem/Abilities/R1GameplayAbility_HitReact.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "R1GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/R1Monster.h"

UR1GameplayAbility_HitReact::UR1GameplayAbility_HitReact(const FObjectInitializer& ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = R1GameplayTags::Event_HitReact;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	ActivationOwnedTags.AddTag(R1GameplayTags::Character_State_HitReact);
}

void UR1GameplayAbility_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AR1Monster* Monster = Cast<AR1Monster>(ActorInfo->AvatarActor.Get());
	if (Monster && Monster->GetCharacterMovement())
	{
		// 피격 중에는 못 움직이게 이동 속도를 0으로! (기존 속도는 어딘가에 저장해두는 것이 좋습니다)
		Monster->GetCharacterMovement()->MaxWalkSpeed = 0.f;
		Monster->GetCharacterMovement()->StopMovementImmediately();
	}

	// OnMontageFinished 함수 안에서:
	if (Monster && Monster->GetCharacterMovement())
	{
		// 애니메이션이 끝나면 원래 속도(예: 300)로 복구!
		Monster->GetCharacterMovement()->MaxWalkSpeed = 300.f;
	}

	if (HitMontage)
	{
		// 1. 몽타주 재생 태스크 생성
		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, HitMontage, 1.f);

		// 2. 몽타주가 끝나거나, 끊기거나, 취소되었을 때 실행할 함수 연결
		Task->OnBlendOut.AddDynamic(this, &UR1GameplayAbility_HitReact::OnMontageFinished);
		Task->OnCompleted.AddDynamic(this, &UR1GameplayAbility_HitReact::OnMontageFinished);
		Task->OnInterrupted.AddDynamic(this, &UR1GameplayAbility_HitReact::OnMontageFinished);
		Task->OnCancelled.AddDynamic(this, &UR1GameplayAbility_HitReact::OnMontageFinished);

		// 3. 태스크 실행! (애니메이션 재생 시작)
		Task->ReadyForActivation();
		UE_LOG(LogTemp, Warning, TEXT("Ouch"));
	}
	else
	{
		// 몽타주가 없으면 바로 어빌리티 종료
		UE_LOG(LogTemp, Error, TEXT("HitReact 어빌리티에 HitMontage가 세팅되지 않았습니다!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}

	
}

void UR1GameplayAbility_HitReact::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
