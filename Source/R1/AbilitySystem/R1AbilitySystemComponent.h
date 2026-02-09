

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

	void ActivateAbility(FGameplayTag AbilityTag);
	
	//TArray<FGameplayAbilitySpecHandle> SpecHandles;
	FGameplayTagContainer AbilityTags;
};
