# Refactor `CachedStatUpgradeData` to Value Types Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Resolve Unreal Header Tool (UHT) compatibility issues by changing the `CachedStatUpgradeData` map value type from `FR1StatUpgradeData*` to `FR1StatUpgradeData` in `UR1RunUpgradeComponent`.

**Architecture:** 
- Modify `R1RunUpgradeComponent.h` to store values instead of pointers in the `TMap`.
- Update `R1RunUpgradeComponent.cpp` to dereference row pointers when caching data and adjust map lookups to handle value types.

**Tech Stack:** C++, Unreal Engine 5 (GAS)

---

### Task 1: Update Header File

**Files:**
- Modify: `Source/R1/Player/R1RunUpgradeComponent.h:74`

- [ ] **Step 1: Modify `CachedStatUpgradeData` declaration**
Change the value type from `struct FR1StatUpgradeData*` to `struct FR1StatUpgradeData`.

```cpp
	UPROPERTY(Transient)
	TMap<FGameplayTag, struct FR1StatUpgradeData> CachedStatUpgradeData;
```

- [ ] **Step 2: Commit header change**

```bash
git add Source/R1/Player/R1RunUpgradeComponent.h
git commit -m "refactor: change CachedStatUpgradeData map value to store by value"
```

### Task 2: Update Source File - Caching Logic

**Files:**
- Modify: `Source/R1/Player/R1RunUpgradeComponent.cpp:30-41`

- [ ] **Step 1: Update `CacheDataTable` to dereference row pointers**
When adding rows from the DataTable to the cache map, dereference the pointer.

```cpp
	for (FR1StatUpgradeData* Row : AllRows)
	{
		if (Row)
		{
			CachedStatUpgradeData.Add(Row->StatTag, *Row);
		}
	}
```

- [ ] **Step 2: Commit caching logic change**

```bash
git add Source/R1/Player/R1RunUpgradeComponent.cpp
git commit -m "refactor: update CacheDataTable to store struct values"
```

### Task 3: Update Source File - Effect Application Logic

**Files:**
- Modify: `Source/R1/Player/R1RunUpgradeComponent.cpp:155-156`

- [ ] **Step 1: Update `ApplyRunUpgradeEffect` map lookup**
Adjust the `Find` result type and member access. `TMap::Find` returns a pointer to the value, so it now returns `FR1StatUpgradeData*` instead of `FR1StatUpgradeData**`.

```cpp
			if (FR1StatUpgradeData* FoundDataPtr = CachedStatUpgradeData.Find(StatTag))
			{
				float TotalBonus = InvestmentCount * FoundDataPtr->IncreaseAmount;
```

- [ ] **Step 2: Final Verification**
Ensure the project compiles (triggering UHT) and the logic for applying upgrades remains correct.

- [ ] **Step 3: Commit effect application change**

```bash
git add Source/R1/Player/R1RunUpgradeComponent.cpp
git commit -m "refactor: update ApplyRunUpgradeEffect to use value-based map"
```
