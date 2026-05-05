#include "Library/R1StatFormattingLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"

FText UR1StatFormattingLibrary::FormatStatValue(float Value, ER1StatDisplayType DisplayType)
{
	switch (DisplayType)
	{
	case ER1StatDisplayType::Integer:
		return FText::AsNumber(FMath::FloorToInt(Value));
	case ER1StatDisplayType::Float:
	{
		FNumberFormattingOptions Options;
		Options.MaximumFractionalDigits = 2;
		Options.MinimumFractionalDigits = 0;
		Options.UseGrouping = false;
		return FText::AsNumber(Value, &Options);
	}
	default:
		return FText::AsNumber(Value);
	}
}

FText UR1StatFormattingLibrary::GetFractionText(float Current, float Max)
{
	return FText::Format(NSLOCTEXT("R1", "FractionFormat", "{0} / {1}"), 
		FText::AsNumber(FMath::FloorToInt(Current)), 
		FText::AsNumber(FMath::FloorToInt(Max)));
}

FText UR1StatFormattingLibrary::GetWeaponDamageRangeText(UAbilitySystemComponent* ASC)
{
	if (!ASC) return FText::GetEmpty();

	float BaseDamage = ASC->GetNumericAttribute(UR1AttributeSet::GetBaseDamageAttribute());
	
	float WeaponDamage = ASC->GetNumericAttribute(UPlayerAttributeSet::GetWeaponDamageAttribute());
	
	float DamageMultiplier = ASC->GetNumericAttribute(UPlayerAttributeSet::GetDamageMultiplierAttribute());

	// Default to 1.0 if multiplier not found or is zero
	if (DamageMultiplier == 0.f) DamageMultiplier = 1.0f;

	float TotalBaseValue = (BaseDamage + WeaponDamage) * DamageMultiplier;
	
	float Min = TotalBaseValue * 0.75f;
	float Max = TotalBaseValue * 1.25f;

	return FText::Format(NSLOCTEXT("R1", "DamageRangeFormat", "{0}  -  {1}"), 
		FText::AsNumber(FMath::FloorToInt(Min)), 
		FText::AsNumber(FMath::FloorToInt(Max)));
}
