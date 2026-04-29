

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "R1GameplayAbility_Attack.generated.h"

class AR1Character;
/**
 * 
 */
UCLASS()
class R1_API UR1GameplayAbility_Attack : public UR1GameplayAbility
{
	GENERATED_BODY()
	
public:
	UR1GameplayAbility_Attack(const FObjectInitializer& ObjectInitializer);

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UGameplayEffect> DamageEffect;

	// [설정] 애니메이션에서 보낼 이벤트 태그 (예: Event.Montage.Hit)
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

private:
	void CheckAndApplyDamage_Sector(const FGameplayEffectSpecHandle& SpecHandle,AR1Character* SourceCharacter, UAbilitySystemComponent* SourceASC);
};
