

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "R1GameplayAbility_LevelUp.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1GameplayAbility_LevelUp : public UR1GameplayAbility
{
	GENERATED_BODY()
public:
	UR1GameplayAbility_LevelUp(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 1. 블루프린트에서 할당할 레벨업 회복(체력/마나 100%) 이펙트 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LevelUp")
	TSubclassOf<class UGameplayEffect> LevelUpRecoveryEffect;

	//// 2. 블루프린트에서 파티클 띄우고 사운드 재생하라고 알려주는 이벤트 (C++ 구현 불필요)
	//UFUNCTION(BlueprintImplementableEvent, Category = "LevelUp")
	//void PlayLevelUpEffects(float NewLevel);
};
