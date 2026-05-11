


#include "AbilitySystem/Abilities/R1GameplayAbility_ChainLightning.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Library/R1AbilitySystemLibrary.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "Character/R1Character.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "System/R1EquipmentManagerComponent.h"
#include "Character/R1Player.h"

UR1GameplayAbility_ChainLightning::UR1GameplayAbility_ChainLightning()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
}

void UR1GameplayAbility_ChainLightning::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!TriggerEventData || !TriggerEventData->Target)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AR1Character* SourceActor = Cast<AR1Character>(GetAvatarActorFromActorInfo());
	AR1Character* InitialTarget = Cast<AR1Character>(const_cast<AActor*>(TriggerEventData->Target.Get()));

	if (!InitialTarget)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 2. 앞서 만든 C++ 헬퍼 함수를 호출하여 튕길 경로(타겟 배열)를 싹 다 계산해옵니다.
	BouncingTargets = UR1AbilitySystemLibrary::GetChainLightningTargets(SourceActor, InitialTarget, BounceRadius, MaxBounces);

	BouncingTargets.Insert(InitialTarget, 0);

	// 3. 상태 초기화 후 첫 번째 튕기기 시작!
	CurrentBounceIndex = 0;
	LastHitActor = InitialTarget; // 최초 시작점은 평타를 맞은 몬스터!

	ProcessNextBounce(); // 루프 시작
}

void UR1GameplayAbility_ChainLightning::ProcessNextBounce()
{
	// 1. 모든 타겟을 다 튕겼는지 확인 (종료 조건)
	if (CurrentBounceIndex >= BouncingTargets.Num())
	{
		// 어빌리티 정상 종료
		bool bReplicateEndAbility = true;
		bool bWasCancelled = false;
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
		return;
	}

	// 2. 현재 때릴 타겟 꺼내기
	AR1Character* TargetActor = BouncingTargets[CurrentBounceIndex];

	if (IsValid(TargetActor))
	{
		// 3. 대상에게 번개 데미지(Gameplay Effect) 적용!
		if (LightningEffect)
		{
			UR1AbilitySystemComponent* ASC = Cast<UR1AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(LightningEffect, GetAbilityLevel(), ContextHandle);

			UR1AbilitySystemComponent* TargetASC = Cast<UR1AbilitySystemComponent>(TargetActor->FindComponentByClass<UAbilitySystemComponent>());
			if (TargetASC && SpecHandle.IsValid())
			{
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}

		if (LightningVFX && LastHitActor && TargetActor)
		{
			FVector StartLocation = LastHitActor->GetActorLocation(); 
			FVector EndLocation = TargetActor->GetActorLocation();

			UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				LightningVFX,
				StartLocation
			);

			if (NiagaraComp)
			{
				NiagaraComp->SetVectorParameter(FName("BeamEnd"), EndLocation);
			}
		}

		// [오디오 트리거] 장착된 무기에서 소리를 가져와 GameplayCue를 실행합니다.
		if (GameplayCueTag.IsValid() && AudioTag.IsValid())
		{
			AActor* SourceActor = GetAvatarActorFromActorInfo();
			UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

			if (SourceActor && SourceASC)
			{
				if (AR1Player* Player = Cast<AR1Player>(SourceActor))
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
					CueParams.Instigator = SourceActor;
					CueParams.Location = TargetActor->GetActorLocation();
					SourceASC->ExecuteGameplayCue(GameplayCueTag, CueParams);
				}
			}
		}

		//DrawDebugLine(GetWorld(), LastHitActor->GetActorLocation(), TargetActor->GetActorLocation(), FColor::Yellow, false, 1.0f, 0, 5.0f);

		// 다음 튕기기를 위해 방금 때린 애를 LastHit으로 기억
		LastHitActor = TargetActor;
	}

	// 5. 인덱스 증가
	CurrentBounceIndex++;

	// 6. 💡 딜레이 후 다시 이 함수를 호출하도록 Task 생성 (C++식 딜레이 루프)
	UAbilityTask_WaitDelay* WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, BounceDelay);
	WaitDelayTask->OnFinish.AddDynamic(this, &UR1GameplayAbility_ChainLightning::ProcessNextBounce);
	WaitDelayTask->ReadyForActivation(); // 태스크 실행!
}
