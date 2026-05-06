# Design Spec: Refactor `CachedStatUpgradeData` to Value Types

**Date:** 2026-05-05
**Topic:** Refactor pointer-based TMap to value-based TMap in `UR1RunUpgradeComponent`.

## Problem
`TMap<FGameplayTag, FR1StatUpgradeData*>` marked with `UPROPERTY` causes Unreal Header Tool (UHT) errors because UHT does not support pointers to structs in reflected containers.

## Proposed Changes

### 1. `R1RunUpgradeComponent.h`
Modify the declaration of `CachedStatUpgradeData` to store `FR1StatUpgradeData` by value instead of by pointer.

```cpp
// Before
UPROPERTY(Transient)
TMap<FGameplayTag, struct FR1StatUpgradeData*> CachedStatUpgradeData;

// After
UPROPERTY(Transient)
TMap<FGameplayTag, struct FR1StatUpgradeData> CachedStatUpgradeData;
```

### 2. `R1RunUpgradeComponent.cpp`

#### `CacheDataTable()`
Dereference the row pointer when adding data to the map.

```cpp
// Before
CachedStatUpgradeData.Add(Row->StatTag, Row);

// After
CachedStatUpgradeData.Add(Row->StatTag, *Row);
```

#### `ApplyRunUpgradeEffect()`
Update the usage of `CachedStatUpgradeData.Find()`. Since the map now stores values, `Find` returns `FR1StatUpgradeData*` directly.

```cpp
// Before
if (FR1StatUpgradeData** FoundDataPtr = CachedStatUpgradeData.Find(StatTag))
{
    float TotalBonus = InvestmentCount * (*FoundDataPtr)->IncreaseAmount;
    // ...
}

// After
if (FR1StatUpgradeData* FoundDataPtr = CachedStatUpgradeData.Find(StatTag))
{
    float TotalBonus = InvestmentCount * FoundDataPtr->IncreaseAmount;
    // ...
}
```

## Testing & Verification
- Verify successful compilation with UHT.
- Run the game and ensure that stat upgrades (investing points) still apply correctly through the Gameplay Effect.
- Check logs for any "Could not find data for tag" warnings that might indicate a regression in map lookups.
