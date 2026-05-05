# Stat Row Getters Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add public getters for attribute name text in stat row widgets and ensure necessary headers are included.

**Architecture:** Add `GetAttributeName()` to `UR1StatDetailRow` and `UR1StatUpgradeRow` and update includes.

**Tech Stack:** C++, Unreal Engine 5 (UMG)

---

### Task 1: Update UR1StatDetailRow

**Files:**
- Modify: `Source/R1/UI/Stat/R1StatDetailRow.h`
- Modify: `Source/R1/UI/Stat/R1StatDetailRow.cpp`

- [ ] **Step 1: Declare getter in header**
```cpp
// ... existing code ...
public:
	void InjectData(const FText& FormattedValue);
	FText GetAttributeName() const;
// ... existing code ...
```

- [ ] **Step 2: Implement getter in source**
```cpp
FText UR1StatDetailRow::GetAttributeName() const
{
	return Text_AttributeName ? Text_AttributeName->GetText() : FText::GetEmpty();
}
```

- [ ] **Step 3: Commit**
```bash
git add Source/R1/UI/Stat/R1StatDetailRow.h Source/R1/UI/Stat/R1StatDetailRow.cpp
git commit -m "refactor: add GetAttributeName to UR1StatDetailRow"
```

### Task 2: Update UR1StatUpgradeRow

**Files:**
- Modify: `Source/R1/UI/Stat/R1StatUpgradeRow.h`
- Modify: `Source/R1/UI/Stat/R1StatUpgradeRow.cpp`

- [ ] **Step 1: Declare getter in header**
```cpp
// ... existing code ...
public:
	void InjectData(int32 InvestmentCount);
	FText GetAttributeName() const;
// ... existing code ...
```

- [ ] **Step 2: Implement getter and add includes in source**
```cpp
#include "UI/Stat/R1StatUpgradeRow.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

// ... existing code ...

FText UR1StatUpgradeRow::GetAttributeName() const
{
	return Text_AttributeName ? Text_AttributeName->GetText() : FText::GetEmpty();
}
```

- [ ] **Step 3: Commit**
```bash
git add Source/R1/UI/Stat/R1StatUpgradeRow.h Source/R1/UI/Stat/R1StatUpgradeRow.cpp
git commit -m "refactor: add GetAttributeName and include Button.h in UR1StatUpgradeRow"
```

### Task 3: Final Verification

- [ ] **Step 1: Verify Build**
Run: `dotnet build` (or relevant UE build command)
Expected: Success

- [ ] **Step 2: Final Commit**
```bash
git commit --allow-empty -m "refactor: add attribute name getters to stat row widgets"
```
