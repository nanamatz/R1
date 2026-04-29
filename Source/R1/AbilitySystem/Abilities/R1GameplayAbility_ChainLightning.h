

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "R1GameplayAbility_ChainLightning.generated.h"

class AR1Character;
/**
 * 
 */
UCLASS()
class R1_API UR1GameplayAbility_ChainLightning : public UR1GameplayAbility
{
	GENERATED_BODY()
public:
	UR1GameplayAbility_ChainLightning();

	// 💡 이벤트(평타 적중)를 통해 실행될 때 호출되는 핵심 함수
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 💡 시간차를 두고 다음 타겟을 타격하는 재귀 함수
	UFUNCTION()
	void ProcessNextBounce();

	UPROPERTY(EditDefaultsOnly, Category = "Lightning")
	float BounceDelay = 0.05f; 

	UPROPERTY(EditDefaultsOnly, Category = "Lightning")
	float BounceRadius = 800.0f; 

	UPROPERTY(EditDefaultsOnly, Category = "Lightning")
	int32 MaxBounces = 8; 

	UPROPERTY(EditDefaultsOnly, Category = "Lightning")
	TSubclassOf<UGameplayEffect> LightningEffect; 

private:
	// 현재 어빌리티 실행에서 기억해둘 상태 변수들
	UPROPERTY()
	TArray<class AR1Character*> BouncingTargets;

	int32 CurrentBounceIndex = 0;

	UPROPERTY()
	AR1Character* LastHitActor = nullptr; // 번개 이펙트(VFX)를 그릴 때 시작점이 될 이전 타겟

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UNiagaraSystem> LightningVFX;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	FGameplayTag AudioTag;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	FGameplayTag GameplayCueTag;
};
