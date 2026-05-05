# Run-Specific Stat Upgrade System Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement a run-specific stat upgrade system that allows players to invest points into stats during a run, ensuring data resets on death and does not contaminate base stats.

**Architecture:** A `UR1RunUpgradeComponent` on `AR1PlayerState` manages points and investment history. It applies bonuses via a `SetByCaller` Gameplay Effect. UI updates are handled through "Data Injection," matching pre-placed widgets by name.

**Tech Stack:** C++, Unreal Engine 5.x, Gameplay Ability System (GAS).

---

### Task 1: Tags and Data Structures

**Files:**
- Modify: `Source/R1/R1GameplayTags.h`
- Modify: `Source/R1/R1GameplayTags.cpp`
- Create: `Source/R1/DataTable/R1StatUpgradeData.h`

- [ ] **Step 1: Define Stat Tags**
Add tags like `Stat.Run.Points`, `Stat.Run.Upgrade.Attack`, `Stat.Run.Upgrade.Health` etc.

- [ ] **Step 2: Implement Stat Tags**
Initialize tags in `R1GameplayTags.cpp`.

- [ ] **Step 3: Create Stat Upgrade Data Struct**
Define `ER1StatDisplayType` and `FR1StatUpgradeData`.

```cpp
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "R1StatUpgradeData.generated.h"

UENUM(BlueprintType)
enum class ER1StatDisplayType : uint8 { Integer, Float, Fraction, Range };

USTRUCT(BlueprintType)
struct FR1StatUpgradeData : public FTableRowBase {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag StatTag;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText StatName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayAttribute Attribute;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float IncreaseAmount;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) ER1StatDisplayType DisplayType;
};
```

- [ ] **Step 4: Commit**
`git add . && git commit -m "feat: add stat upgrade tags and data structures"`

---

### Task 2: Run Upgrade Component (Logic)

**Files:**
- Create: `Source/R1/Player/R1RunUpgradeComponent.h`
- Create: `Source/R1/Player/R1RunUpgradeComponent.cpp`

- [ ] **Step 1: Define UR1RunUpgradeComponent**
Implement point management, investment history, and GE application logic.

- [ ] **Step 2: Implement UpgradeStat Function**
Update history, deduct points, and refresh `SetByCaller` magnitudes in the active GE.

- [ ] **Step 3: Commit**
`git add . && git commit -m "feat: implement UR1RunUpgradeComponent logic"`

---

### Task 3: PlayerState Integration

**Files:**
- Modify: `Source/R1/Player/R1PlayerState.h`
- Modify: `Source/R1/Player/R1PlayerState.cpp`

- [ ] **Step 1: Add Component to PlayerState**
Create the component in the constructor.

- [ ] **Step 2: Listen for Level Changes**
Bind a callback to `UPlayerAttributeSet::Level` and grant points on increase.

- [ ] **Step 3: Handle Reset**
Ensure the component resets its data in `BeginPlay`.

- [ ] **Step 4: Commit**
`git add . && git commit -m "feat: integrate RunUpgradeComponent into PlayerState"`

---

### Task 4: UI Formatting and Range Logic

**Files:**
- Create: `Source/R1/Library/R1StatFormattingLibrary.h`
- Create: `Source/R1/Library/R1StatFormattingLibrary.cpp`

- [ ] **Step 1: Implement Formatting Logic**
Create a utility to format values based on `ER1StatDisplayType`.

- [ ] **Step 2: Implement Weapon Damage Range Calculation**
`BaseValue = (BaseDamage + WeaponDamage) * DamageMultiplier`. Return `{BaseValue * 0.75} ~ {BaseValue * 1.25}`.

- [ ] **Step 3: Commit**
`git add . && git commit -m "feat: add stat formatting and range calculation library"`

---

### Task 5: UI Row Data Injection

**Files:**
- Modify: `Source/R1/UI/Stat/R1StatUpgradeRow.h/cpp`
- Modify: `Source/R1/UI/Stat/R1StatDetailRow.h/cpp`

- [ ] **Step 1: Implement Injection in UpgradeRow**
Add `InjectData(int32 InvestmentCount)` to update `Text_StatValue`.

- [ ] **Step 2: Implement Injection in DetailRow**
Add `InjectData(const FText& FormattedValue)` to update `Text_Amount`.

- [ ] **Step 3: Commit**
`git add . && git commit -m "feat: add data injection methods to stat row widgets"`

---

### Task 6: Main Stat UI Injection Logic

**Files:**
- Modify: `Source/R1/UI/Stat/R1CharacterStatUI.h/cpp`

- [ ] **Step 1: Implement RefreshUI**
Iterate through `ScrollBox_UpgradeList` and `ScrollBox_DetailList`. Match rows by `Text_AttributeName` and inject updated data.

- [ ] **Step 2: Update Available Points Text**
Update `Text_RemainPointAmount` at the bottom.

- [ ] **Step 3: Final Verification**
Run the game, level up, spend points, and verify UI reflects the investment count and final calculated stats.

- [ ] **Step 4: Commit**
`git add . && git commit -m "feat: implement main stat UI injection and verification"`
