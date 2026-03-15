
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility_Attack.h"
#include "R1GameplayAbility_RangedAttack.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1GameplayAbility_RangedAttack : public UR1GameplayAbility_Attack
{
	GENERATED_BODY()

protected:
    virtual void OnAttackEventReceived(FGameplayEventData Payload) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<class UParticleSystem> FiredEffect;
};
