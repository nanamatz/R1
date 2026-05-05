# Meta Progression Bug Fix Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix the bug where meta-level and skill points disappear or fail to increase by ensuring Level/Exp synchronization happens correctly at the start of a run.

**Architecture:** 
- Align default level values across Attribute Sets and Save Games.
- Ensure Level/Exp synchronization in `AR1PlayerState` occurs regardless of whether a player has purchased meta-upgrades.

**Tech Stack:** C++, Unreal Engine 5 (GAS/SaveSystem)

---

### Task 1: Update Default Initial Values

**Files:**
- Modify: `Source/R1/AbilitySystem/Attribute/PlayerAttributeSet.cpp:18`
- Modify: `Source/R1/System/R1MetaSaveGame.h:23-27`

- [ ] **Step 1: Update Attribute Set default level**
Change `InitLevel(0.f)` to `InitLevel(1.f)` to match the save game's starting level.

- [ ] **Step 2: Update Meta Save Game defaults**
Set `CurrentMetaExp` and `AvailableSkillPoints` to 0 by default so new players don't start with unearned progress.

- [ ] **Step 3: Commit initial value changes**

### Task 2: Fix `ApplyMetaUpgrades` Logic Flow

**Files:**
- Modify: `Source/R1/Player/R1PlayerState.cpp:87-120`

- [ ] **Step 1: Move synchronization logic before the early return**
Ensure that the player's Attribute Set is updated with the Meta Save's Level and Exp even if they haven't bought any upgrades yet.

- [ ] **Step 2: Commit logic flow fix**

### Task 3: Final Verification

- [ ] **Step 1: Verify data persistence**
1. Start the game.
2. Gain experience and level up in a run (e.g., reach Level 2).
3. End the run (die or clear).
4. Verify that `PlayerMetaLevel` is now 2 and you have 1 `AvailableSkillPoint` (if points are 1 per level).
5. Start a NEW run and verify that the UI shows Level 2 immediately.

- [ ] **Step 2: Commit verification results (if applicable)**
