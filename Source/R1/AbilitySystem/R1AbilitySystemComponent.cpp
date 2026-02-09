


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

void UR1AbilitySystemComponent::ActivateAbility(FGameplayTag AbilityTag)
{
	//for (FGameplayAbilitySpecHandle& SpecHandle : SpecHandles)
	//{
	//	//TODO
	//	TryActivateAbility(SpecHandle);
	//}
	// 1. 네임스페이스에서 전달받은 태그(예: R1GameplayTags::Ability_Attack)를 컨테이너에 담습니다.

	AbilityTags.AddTag(AbilityTag);

	// 2. GAS 표준 함수 호출: 이 태그를 'Ability Tags' 섹션에 포함한 모든 어빌리티를 실행 시도합니다.
	TryActivateAbilitiesByTag(AbilityTags);

}
