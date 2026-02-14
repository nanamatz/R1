


#include "AbilitySystem/Abilities/R1GameplayAbility_LevelUp.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/R1Player.h"
#include "R1GameplayAbility_LevelUp.h"

UR1GameplayAbility_LevelUp::UR1GameplayAbility_LevelUp(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
}


void UR1GameplayAbility_LevelUp::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AR1Player* R1Player = Cast<AR1Player>(ActorInfo->AvatarActor);

	if (R1Player)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(R1Player);
		if (ASC && LevelUpRecoveryEffect) 
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddInstigator(R1Player, R1Player);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(LevelUpRecoveryEffect, 1.0f, ContextHandle);

			// 4. 플레이어에게 GE를 적용합니다!
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	
	//if (CommitAbility(Handle, ActorInfo, ActivationInfo))
	//{
	//	// 1. C++ 코어 로직: 체력/마나 회복 GE 적용
	//	if (LevelUpRecoveryEffect)
	//	{
	//		// 자신에게 즉시 GE 적용
	//		ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, LevelUpRecoveryEffect.GetDefaultObject(), 1.0f);
	//	}

	//	// 2. 블루프린트 로직 호출: 전달받은 새 레벨(EventMagnitude)을 넘겨주며 연출 재생
	//	//float NewLevel = TriggerEventData ? TriggerEventData->EventMagnitude : 0.0f;
	//	//PlayLevelUpEffects(NewLevel);
	//}

	// 레벨업 어빌리티가 끝났음을 알림
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

