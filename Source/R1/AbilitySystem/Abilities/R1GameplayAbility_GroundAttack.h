#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility_BossAttackBase.h"
#include "R1GameplayAbility_GroundAttack.generated.h"

class UGameplayEffect;

UCLASS()
class R1_API UR1GameplayAbility_GroundAttack : public UR1GameplayAbility_BossAttackBase
{
	GENERATED_BODY()

protected:
	virtual void OnAttackEventReceived(FGameplayEventData Payload) override;

	UPROPERTY(EditAnywhere, Category = "Attack")
	TSubclassOf<UGameplayEffect> DamageEffect;
};
