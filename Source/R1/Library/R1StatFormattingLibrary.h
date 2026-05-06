#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DataTable/R1StatUpgradeData.h"
#include "R1StatFormattingLibrary.generated.h"

class UAbilitySystemComponent;

/**
 * Library for formatting stat values and calculating complex stat strings for the UI.
 */
UCLASS()
class R1_API UR1StatFormattingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Formats a single float value based on the display type. */
	UFUNCTION(BlueprintPure, Category = "R1|Formatting")
	static FText FormatStatValue(float Value, ER1StatDisplayType DisplayType);

	/** Formats a fraction (e.g., "100 / 150"). */
	UFUNCTION(BlueprintPure, Category = "R1|Formatting")
	static FText GetFractionText(float Current, float Max);

	/** Calculates and formats the weapon damage range (e.g., "75 ~ 125"). */
	UFUNCTION(BlueprintPure, Category = "R1|Formatting")
	static FText GetWeaponDamageRangeText(UAbilitySystemComponent* ASC);
};
