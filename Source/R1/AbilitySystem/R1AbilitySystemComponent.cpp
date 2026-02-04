


#include "AbilitySystem/R1AbilitySystemComponent.h"

void UR1AbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<class UGameplayAbility>> Abilities)
{
	for (auto& AbilityClass : Abilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		FGameplayAbilitySpecHandle SpecHandle = GiveAbility(AbilitySpec);

		auto& a = ActivatableAbilities;

		//TryActivateAbility(SpecHandle);
		//GiveAbilityAndActivateOnce(AbilitySpec);
		SpecHandles.Add(SpecHandle);
	}
}

void UR1AbilitySystemComponent::ActivateAbility(FGameplayTag AbilityTag)
{
	for (FGameplayAbilitySpecHandle& SpecHandle : SpecHandles)
	{
		//TODO
		TryActivateAbility(SpecHandle);
	}
}
