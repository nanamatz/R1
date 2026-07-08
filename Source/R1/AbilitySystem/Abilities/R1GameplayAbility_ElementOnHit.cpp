
#include "AbilitySystem/Abilities/R1GameplayAbility_ElementOnHit.h"
#include "R1LogChannels.h"
#include "R1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UR1GameplayAbility_ElementOnHit::UR1GameplayAbility_ElementOnHit(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Ability.Attack 게임플레이 이벤트로 자동 활성화 (별도 활성화 호출 불필요)
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = R1GameplayTags::Ability_Attack;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UR1GameplayAbility_ElementOnHit::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 대상이 이미 죽어 ASC가 무효한 경우 등은 조용히 종료
	AActor* TargetActor = (TriggerEventData != nullptr) ? const_cast<AActor*>(TriggerEventData->Target.Get()) : nullptr;
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	if (OnHitEffect && SourceASC && TargetASC)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(OnHitEffect, 1, EffectContext);
		if (EffectSpecHandle.IsValid() && EffectSpecHandle.Data.IsValid())
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);
		}
	}
	else if (!OnHitEffect)
	{
		UE_LOG(LogR1, Warning, TEXT("[%s] OnHitEffect가 설정되지 않았습니다."), *GetName());
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
