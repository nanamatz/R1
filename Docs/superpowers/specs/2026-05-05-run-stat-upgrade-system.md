# Run-Specific Stat Upgrade System Design

## 1. Overview
The Run-Specific Stat Upgrade System allows players to invest points acquired during a single game session (a "Run") into various character stats. This system is designed to be completely independent of permanent meta-progression and base character stats, ensuring that all bonuses are reset upon death or the start of a new run.

### Core Principles
1.  **No Base Stat Contamination:** Run-specific bonuses are applied as a separate layer using Gameplay Effects with `SetByCaller` magnitudes.
2.  **Data Volatility:** All investment data is stored in a component attached to the `AR1PlayerState`, ensuring it resets when a new run begins.
3.  **UI Layout Preservation (Data Injection):** The system will not dynamically spawn UI elements. Instead, it will search for pre-placed UI items in the editor and "inject" data by matching stat names.

---

## 2. Architecture

### 2.1 Data Structures
*   **`ER1StatDisplayType` (Enum):** Defines how stat values are formatted in the UI (`Integer`, `Float`, `Fraction`, `Range`).
*   **`FR1StatUpgradeData` (Struct/DataTable):**
    *   `StatTag`: `FGameplayTag` identifier.
    *   `StatName`: `FText` (e.g., "Attack Power") used for UI matching.
    *   `Attribute`: `FGameplayAttribute` to modify.
    *   `IncreaseAmount`: Value added per point.
    *   `DisplayType`: `ER1StatDisplayType`.

### 2.2 Logic Component: `UR1RunUpgradeComponent`
Attached to `AR1PlayerState`.
*   **Properties:**
    *   `AvailablePoints`: `int32`.
    *   `InvestmentHistory`: `TMap<FGameplayTag, int32>`.
    *   `RunUpgradeGEHandle`: `FActiveGameplayEffectHandle`.
*   **Functions:**
    *   `AddPoints(int32 Amount)`: Called on level up.
    *   `UpgradeStat(FGameplayTag StatTag)`: Consumes 1 point, updates history, and refreshes the Gameplay Effect.
    *   `Reset()`: Clears history and points.
    *   `GetInvestmentCount(FGameplayTag StatTag)`: Returns how many times a stat has been upgraded.

---

## 3. UI Data Injection

### 3.1 `R1CharacterStatUI` Workflow
The UI will refresh its data by iterating through the `ScrollBox_UpgradeList` and `ScrollBox_DetailList`.

1.  **Find Rows:** Iterate using `GetChildrenCount()` and `GetChildAt(i)`.
2.  **Match Name:** Compare the child's `Text_AttributeName` with the `StatName` from the Data Table.
3.  **Inject Data:**
    *   **Upgrade Row:** Set `Text_StatValue` to the investment count from `UR1RunUpgradeComponent`.
    *   **Detail Row:** Set `Text_Amount` to the calculated final value.

### 3.2 Value Formatting Rules
*   **Integer:** `FloorToInt(Value)`.
*   **Float:** Formatted to 2 decimal places.
*   **Fraction:** `Current / Max` (e.g., Health/MaxHealth).
*   **Range (Weapon Damage):** 
    *   `BaseValue = (BaseDamage + WeaponDamage) * DamageMultiplier`
    *   `Min = BaseValue * 0.75`
    *   `Max = BaseValue * 1.25`
    *   Display: `{Min} ~ {Max}`

---

## 4. Lifecycle & Integration

### 4.1 Acquisition
The `AR1PlayerState` will listen to `UPlayerAttributeSet::Level` changes.
*   Whenever Level increases, call `UR1RunUpgradeComponent::AddPoints(5)`.

### 4.2 Reset
*   `UR1RunUpgradeComponent::BeginPlay` will initialize/reset the run data.
*   Death or "New Run" from the menu will trigger a fresh initialization of the `PlayerState` or a manual call to `Reset()`.

---

## 5. Verification Plan
1.  **Stat Logic:** Verify that upgrading "Attack Power" increases damage in combat without changing the character's base `BaseDamage` attribute permanently.
2.  **Volatility:** Confirm that restarting the game or dying resets points and investment history to zero.
3.  **UI Integrity:** Verify that the UI labels remain exactly as set in the editor, with only the numerical values being updated.
4.  **Formatting:** Confirm "Weapon Damage" displays as a range (e.g., `75 ~ 125`) based on the calculated multipliers.
