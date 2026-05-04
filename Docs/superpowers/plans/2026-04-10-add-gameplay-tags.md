# Add New Gameplay Tags Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add gameplay tags for Critical Hit Chance, Critical Hit Multiplier, and Critical Hit Events to support a new critical hit system.

**Architecture:** Use `UE_DECLARE_GAMEPLAY_TAG_EXTERN` in the header and `UE_DEFINE_GAMEPLAY_TAG` in the source file within the `R1GameplayTags` namespace, following existing patterns in the codebase.

**Tech Stack:** Unreal Engine 5 (Gameplay Tags system), C++.

---

### Task 1: Declare tags in header

**Files:**
- Modify: `Source/R1/R1GameplayTags.h`

- [ ] **Step 1: Add Critical Hit Attribute tags**
Add `Data_Attribute_CriticalHitChance` and `Data_Attribute_CriticalHitMultiplier` near other common attributes.

```cpp
// Source/R1/R1GameplayTags.h

// ... near other Data_Attribute tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_AttackSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_CriticalHitChance);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_CriticalHitMultiplier);
// ...
```

- [ ] **Step 2: Add Critical Hit Event tag**
Add `Event_Hit_Critical` near `Event_HitReact`.

```cpp
// Source/R1/R1GameplayTags.h

// ... near Event_HitReact
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Hit_Critical);
// ...
```

### Task 2: Define tags in cpp

**Files:**
- Modify: `Source/R1/R1GameplayTags.cpp`

- [ ] **Step 1: Define Critical Hit Attribute tags**
Define `Data_Attribute_CriticalHitChance` and `Data_Attribute_CriticalHitMultiplier`.

```cpp
// Source/R1/R1GameplayTags.cpp

// ... near other Data_Attribute definitions
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_AttackSpeed, "Data.Attribute.AttackSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_CriticalHitChance, "Data.Attribute.CriticalHitChance");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_CriticalHitMultiplier, "Data.Attribute.CriticalHitMultiplier");
// ...
```

- [ ] **Step 2: Define Critical Hit Event tag**
Define `Event_Hit_Critical`.

```cpp
// Source/R1/R1GameplayTags.cpp

// ... near Event_HitReact definition
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReact, "Event.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Event_Hit_Critical, "Event.Hit.Critical");
// ...
```

### Task 3: Verification and Commit

- [ ] **Step 1: Verify syntax**
Ensure no syntax errors in the added lines. Since this is Unreal Engine code, full compilation check might be slow, but basic syntax verification is possible.

- [ ] **Step 2: Commit changes**

```bash
git add Source/R1/R1GameplayTags.h Source/R1/R1GameplayTags.cpp
git commit -m "feat: add gameplay tags for critical hits"
```
