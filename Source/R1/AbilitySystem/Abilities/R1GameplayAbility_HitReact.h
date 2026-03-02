

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "R1GameplayAbility_HitReact.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1GameplayAbility_HitReact : public UR1GameplayAbility
{
	GENERATED_BODY()
public:
	UR1GameplayAbility_HitReact(const FObjectInitializer& ObjectInitializer);
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<class UAnimMontage> HitMontage;

	// 몽타주 재생이 끝났을 때 어빌리티를 종료할 콜백 함수들
	UFUNCTION()
	void OnMontageFinished();
};
