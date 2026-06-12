
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "R1GameplayAbility_MonsterComboAttack.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1GameplayAbility_MonsterComboAttack : public UR1GameplayAbility
{
	GENERATED_BODY()
	
public:
	UR1GameplayAbility_MonsterComboAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Data")
	FName SkillID = FName("MonsterComboAttack");

protected:
	UPROPERTY()
	int32 ComboIndex = 0;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<class UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Damage")
	FGameplayTag AttackEventTag;

	virtual void OnMontageEnded() override;
	virtual void OnAttackEventReceived(FGameplayEventData Payload) override;
};
