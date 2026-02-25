


#include "AbilitySystem/R1AbilitySystemComponent.h"

void UR1AbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<class UGameplayAbility>> Abilities)
{
	for (const auto& AbilityClass : Abilities)
	{
		if (AbilityClass)
		{
			// 1. Spec 생성 (레벨 1로 설정)
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

			// 2. 어빌리티 부여 (이 함수가 내부적으로 ActivatableAbilities에 추가함)
			UE_LOG(LogTemp, Warning, TEXT("✅ [Monster] 피격 어빌리티 부여 성공!"));
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
	//for (FGameplayAbilitySpecHandle& SpecHandle : SpecHandles)
	//{
	//	//TODO
	//	TryActivateAbility(SpecHandle);
	//}

	FGameplayTagContainer LocalTagContainer;
	LocalTagContainer.AddTag(AbilityTag);
	TryActivateAbilitiesByTag(LocalTagContainer);

}
