


#include "Player/R1PlayerState.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1PlayerAttributeSet.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"

AR1PlayerState::AR1PlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UR1AbilitySystemComponent>("AbilitySystemComponent");

	PlayerAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>("PlayerAttributeSet");
	CoreAttributeSet = CreateDefaultSubobject<UR1AttributeSet>("CoreAttributeSet");

}


UAbilitySystemComponent* AR1PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UR1AbilitySystemComponent* AR1PlayerState::GetR1AbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UPlayerAttributeSet* AR1PlayerState::GetPlayerAttributeSet() const
{
	return PlayerAttributeSet;
}

UR1AttributeSet* AR1PlayerState::GetCommonAttributeSet() const
{
	return CoreAttributeSet;
}

float AR1PlayerState::GetCurrentExpRatio() const
{
	if (AbilitySystemComponent)
	{
		float Exp = AbilitySystemComponent->GetNumericAttribute(PlayerAttributeSet->GetExpAttribute());
		float MaxExp = AbilitySystemComponent->GetNumericAttribute(PlayerAttributeSet->GetMaxExpAttribute());
		if (MaxExp > 0)
		{
			return Exp / MaxExp;
		}
	}
	return 0.f;
}
