


#include "AbilitySystem/R1AbilitySystemComponent.h"

void UR1AbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<class UGameplayAbility>> Abilities)
{
	for (const auto& AbilityClass : Abilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

			GiveAbility(AbilitySpec);
		}
	}
}

void UR1AbilitySystemComponent::ApplyCharacterEffects(const TArray<TSubclassOf<class UGameplayEffect>> Effects)
{
	for(const auto& EffectClass : Effects)
	{
		if (EffectClass)
		{
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(EffectClass, 1, MakeEffectContext());
			if (SpecHandle.IsValid())
			{
				ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

void UR1AbilitySystemComponent::InitializeCharacterAttributes()
{
}

void UR1AbilitySystemComponent::ActivateAbility(FGameplayTag AbilityTag)
{
	FGameplayTagContainer LocalTagContainer;
	LocalTagContainer.AddTag(AbilityTag);
	TryActivateAbilitiesByTag(LocalTagContainer);

}
