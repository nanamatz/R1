#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "R1GameplayAbility_BossAttackBase.generated.h"

class UR1TelegraphData;
class AR1TelegraphActor;

UCLASS()
class R1_API UR1GameplayAbility_BossAttackBase : public UR1GameplayAbility
{
	GENERATED_BODY()

public:
	UR1GameplayAbility_BossAttackBase();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	virtual void OnAttackEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageEnded();

protected:
	UPROPERTY(EditAnywhere, Category = "Telegraph")
	TObjectPtr<UR1TelegraphData> TelegraphData;

	UPROPERTY(EditAnywhere, Category = "Telegraph")
	TSubclassOf<AR1TelegraphActor> TelegraphActorClass;

	UPROPERTY(EditAnywhere, Category = "Attack")
	FGameplayTag AttackEventTag;
};
