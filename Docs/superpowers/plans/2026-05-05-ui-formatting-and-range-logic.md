# UI Formatting and Range Logic Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement a utility library for formatting stat values and calculating weapon damage ranges for the UI.

**Architecture:** Create a `UBlueprintFunctionLibrary` called `UR1StatFormattingLibrary` with static methods for formatting and range calculation.

**Tech Stack:** C++, Unreal Engine 5, Gameplay Ability System (GAS).

---

### Task 1: Create UR1StatFormattingLibrary Header

**Files:**
- Create: `Source/R1/Library/R1StatFormattingLibrary.h`

- [ ] **Step 1: Write the header file**

```cpp
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
```

- [ ] **Step 2: Commit**

```bash
git add Source/R1/Library/R1StatFormattingLibrary.h
git commit -m "feat: add UR1StatFormattingLibrary header"
```

### Task 2: Implement UR1StatFormattingLibrary

**Files:**
- Create: `Source/R1/Library/R1StatFormattingLibrary.cpp`

- [ ] **Step 1: Write the implementation file**

```cpp
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

	bool bFoundBase = false;
	float BaseDamage = ASC->GetNumericAttribute(UR1AttributeSet::GetBaseDamageAttribute(), bFoundBase);
	
	bool bFoundWeapon = false;
	float WeaponDamage = ASC->GetNumericAttribute(UPlayerAttributeSet::GetWeaponDamageAttribute(), bFoundWeapon);
	
	bool bFoundMult = false;
	float DamageMultiplier = ASC->GetNumericAttribute(UPlayerAttributeSet::GetDamageMultiplierAttribute(), bFoundMult);

	// Default to 1.0 if multiplier not found or is zero (though it should be 1.0)
	if (!bFoundMult || DamageMultiplier == 0.f) DamageMultiplier = 1.0f;

	float TotalBaseValue = (BaseDamage + WeaponDamage) * DamageMultiplier;
	
	float Min = TotalBaseValue * 0.75f;
	float Max = TotalBaseValue * 1.25f;

	return FText::Format(NSLOCTEXT("R1", "DamageRangeFormat", "{0} ~ {1}"), 
		FText::AsNumber(FMath::FloorToInt(Min)), 
		FText::AsNumber(FMath::FloorToInt(Max)));
}
```

- [ ] **Step 2: Commit**

```bash
git add Source/R1/Library/R1StatFormattingLibrary.cpp
git commit -m "feat: implement UR1StatFormattingLibrary"
```

### Task 3: Verify Compilation

- [ ] **Step 1: Run build**

```bash
# Example build command, adjust for project environment
# In this environment, we might not have a full build system accessible, 
# but we should ensure the code is syntactically correct.
```

- [ ] **Step 2: Commit final changes**
