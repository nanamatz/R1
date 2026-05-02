

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "R1GameplayAbility_FistAttack.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1GameplayAbility_FistAttack : public UR1GameplayAbility
{
	GENERATED_BODY()
	
public:
	UR1GameplayAbility_FistAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Data")
	FName SkillID = FName("FistAttack");

protected:
	UPROPERTY(EditAnywhere, Category = "FistAttack")
	TObjectPtr<class UAnimMontage> FistMontage;

	UPROPERTY(EditAnywhere, Category = "FistAttack")
	TSubclassOf<class UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Damage")
	FGameplayTag AttackEventTag;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Audio")
	FGameplayTag AudioTag;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Audio")
	FGameplayTag GameplayCueTag;

	UFUNCTION()
	virtual void OnMontageEnded();

	UFUNCTION()
	virtual void OnAttackEventReceived(FGameplayEventData Payload);
};
