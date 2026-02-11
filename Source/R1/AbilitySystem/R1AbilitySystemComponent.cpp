


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
			GiveAbility(AbilitySpec);
		}
	}

	// 기존의 SpecHandles.Add(SpecHandle)는 삭제해도 무방합니다.
	// GAS 내부에서 태그를 검색할 때 자동으로 ActivatableAbilities를 뒤져서 찾기 때문입니다.
	//for (auto& AbilityClass : Abilities)
	//{
	//	FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
	//	FGameplayAbilitySpecHandle SpecHandle = GiveAbility(AbilitySpec);

	//	auto& a = ActivatableAbilities;

	//	//TryActivateAbility(SpecHandle);
	//	//GiveAbilityAndActivateOnce(AbilitySpec);
	//	SpecHandles.Add(SpecHandle);
	//}
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

	AbilityTags.AddTag(AbilityTag);

	TryActivateAbilitiesByTag(AbilityTags);

}
