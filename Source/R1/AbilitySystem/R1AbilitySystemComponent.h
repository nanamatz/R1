

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "R1AbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	void AddCharacterAbilities(const TArray<TSubclassOf<class UGameplayAbility>> Abilities);
	void ApplyCharacterEffects(const TArray<TSubclassOf<class UGameplayEffect>> Effects);
	void InitializeCharacterAttributes();

	void ActivateAbility(FGameplayTag AbilityTag);
protected:
	//UPROPERTY()
	//TArray<FGameplayAbilitySpecHandle> SpecHandles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FGameplayTagContainer AbilityTags;
};
