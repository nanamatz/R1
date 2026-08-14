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

	virtual void OnAttackEventReceived(FGameplayEventData Payload) override;
	virtual void OnMontageEnded() override;

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Data")
	FName SkillID;

	UPROPERTY(EditDefaultsOnly, Category = "Skill Data")
	TSubclassOf<class UGameplayEffect> CostEffectClass;

	float CachedDamage = 0.0f;
	float CachedManaCost = 0.0f;
	float CachedCooldown = 0.0f;
	float CachedRange = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Telegraph")
	TObjectPtr<UR1TelegraphData> TelegraphData;

	UPROPERTY(EditAnywhere, Category = "Telegraph")
	TSubclassOf<AR1TelegraphActor> TelegraphActorClass;

	UPROPERTY(EditAnywhere, Category = "Attack")
	FGameplayTag AttackEventTag;
};
