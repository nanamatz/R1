# R1 Project Portfolio Document Specification

## Overview
This document serves as the specification for writing a 5,000-word technical portfolio based on the R1 Project. The target audience is Technical Directors and Lead Programmers. The portfolio will use a "Refactoring Journey" narrative, demonstrating how initial monolithic systems were refactored into scalable, data-driven architectures.

## Target Audience & Tone
- **Audience:** Technical Directors / Lead Programmers.
- **Tone:** Professional, highly technical, analytical, and problem-solving oriented.
- **Focus:** Architecture, memory management, C++ implementation details (including code snippets), optimization techniques, and Object-Oriented principles.

## Document Structure & Word Count Allocation (Target: 5,000 words)

### 1. Executive Summary & Project Overview (~500 words)
- **Content:** Introduction to R1 (Action RPG with procedural dungeons).
- **Tech Stack:** Unreal Engine, C++, Gameplay Ability System (GAS).
- **Goal:** Set the stage for the engineering challenges ahead.

### 2. Phase 1: Prototyping & The Monolithic Trap (~700 words)
- **Content:** Discussing the early development phase where logic was tightly coupled.
- **Problem:** Tightly coupled logic, monolithic character classes leading to inflexible design.
- **Solution:** Identifying the bottlenecks that necessitated a data-driven approach, paving the way for the core subsystems.

### 3. Phase 2: Procedural World & The Memory Crisis (~1,000 words)
- **Content:** Deep dive into map generation and memory management.
- **Problem:** Synchronous loading of massive dungeon maps caused severe frame drops.
- **Refactor:** Implementing `AR1MapGenerator` using a queue-based Isaac-style branching algorithm.
- **Optimization:** Designing the `UR1RoomStreamingSubsystem`. Explaining the "Thermal State" machine (Hot, Warm, Cold, Preloading) and its impact on memory budgets (`FR1RuntimeBudget`).

### 4. Phase 3: Scaling Combat with GAS (~1,000 words)
- **Content:** Transitioning to Unreal's Gameplay Ability System.
- **Problem:** Hardcoded combat abilities became unmaintainable as enemy types increased.
- **Refactor:** Integrating `UR1AbilitySystemComponent` and `UR1AttributeSet`.
- **Optimization:** Establishing a data-driven pipeline using PrimaryDataAssets to define gameplay behavior via Gameplay Tags, GAs, and GEs.

### 5. Phase 4: The Grid Inventory Optimization (~1,000 words)
- **Content:** Architecting a Tetris-style grid inventory without performance loss.
- **Problem:** Spawning an `AActor` for every item in the inventory crushed performance.
- **Refactor:** Separating visual representation (`AR1ItemActor`) from logical data (`UR1ItemInstance`).
- **Optimization:** Implementing O(1) mathematical grid checks (`CanAddItemAt`) within `UR1InventorySubsystem` and managing active gameplay effects via `UR1EquipmentManagerComponent` (Receipt Pattern).

### 6. Conclusion & Lessons Learned (~800 words)
- **Content:** Final performance metrics, key architectural takeaways (Subsystems, Data Assets, Async Loading), and reflections on scalable game development.

## Implementation Guidelines
- The portfolio should be written in Markdown (`Portfolio.md`).
- Code snippets from the actual project must be embedded to support technical claims (e.g., `ER1RoomThermalState` enum, `CanAddItemAt` logic).
- Transition smoothly between sections to maintain the chronological narrative arc.

## Review Check
- [x] Placeholders removed.
- [x] Internal consistency verified.
- [x] Scope is well-defined and focused.
- [x] Ambiguity addressed.
