
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility_Attack.h"
#include "R1GameplayAbility_MonsterMeeleAttack.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1GameplayAbility_MonsterMeeleAttack : public UR1GameplayAbility_Attack
{
	GENERATED_BODY()

public:
	UR1GameplayAbility_MonsterMeeleAttack(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnAttackEventReceived(FGameplayEventData Payload) override;
};
