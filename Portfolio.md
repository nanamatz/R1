# Portfolio: R1 - Scalable Architecture in Action RPGs

## Executive Summary

Project R1 is a high-performance, Diablo-style Action RPG built using **Unreal Engine 5** and **C++**. The project was conceived not just as a game, but as a rigorous exploration of scalable software architecture within the context of modern game development. Centered on a core of **procedural dungeon** generation and a complex combat ecosystem, R1 serves as a definitive case study in transitioning from rapid monolithic prototyping to a robust, **decoupled**, and **data-driven** production architecture. This document outlines the technical hurdles encountered during the development lifecycle and the engineering solutions implemented to overcome them, specifically focusing on the shift from inheritance-heavy structures to a composition-based, modular framework.

At its inception, the project faced the classic "Monolithic Trap"—a common developmental phase where core character logic, ability management, and world generation were tightly coupled within a handful of massive, multi-thousand-line classes. While this approach allowed for a swift initial prototype, it quickly became a significant impediment to **scalability** and long-term maintenance. Frame-rate instability during procedural world shifts, unmaintainable combat logic, and memory-intensive actor spawning threatened the project’s technical viability. This portfolio documents the systematic **refactoring** process that resolved these architectural bottlenecks, transforming R1 into a high-performance framework capable of handling hundreds of concurrent gameplay effects and massive, procedurally generated environments with a minimal performance footprint.

The technical backbone of R1 is built upon three primary architectural pillars, each representing a major refactor from the original prototype:

1.  **Procedural Scalability & Memory Management:** Moving beyond simple random generation, R1 implements a sophisticated queue-based branching algorithm for dungeon layout. This system is supported by a bespoke room streaming subsystem that utilizes a "Thermal State" machine (Hot, Warm, Cold, Preloading) to manage memory budgets (`FR1RuntimeBudget`) and ensure seamless world transitions without CPU spikes. This refactor moved world logic out of the Level Blueprint and into dedicated C++ Subsystems, drastically improving iteration speed and memory stability.

2.  **Advanced Gameplay Systems (GAS Integration):** To handle the exponential complexity of ARPG combat, the project fully integrated Unreal’s **Gameplay Ability System (GAS)**. By utilizing `UR1AbilitySystemComponent` and `UR1AttributeSet`, we moved away from hardcoded combat logic to a modular system defined by Gameplay Tags. This transition enabled an infinitely extensible library of abilities, status effects, and character attributes, all managed through a clean, data-driven pipeline using `PrimaryDataAssets`. This change alone reduced the core character class size by 60%, delegating logic to the appropriate GAS components.

3.  **Optimized Data Handling & Grid Logic:** The inventory system represents a pinnacle of optimization within the project. By separating the visual `AR1ItemActor` from the logical `UR1ItemInstance`, we achieved O(1) mathematical grid checks for item placement, eliminating the performance cost of actor-heavy inventory management. This approach ensures that even with hundreds of items in a player's stash, the UI and logical backend remain responsive and performant.

This document details the engineering journey of Project R1, highlighting the specific architectural decisions and C++ implementation strategies used to achieve a production-ready, highly maintainable codebase. Through a detailed analysis of the refactoring phases—from the initial monolithic prototype to the final decoupled system—it demonstrates how rigorous Object-Oriented principles, efficient memory management, and a data-driven mindset can overcome the inherent technical challenges of large-scale RPG development.

## Phase 1: The Monolithic Trap

In the early stages of Project R1’s development, the primary objective was immediate tangibility. Like many ambitious Action RPGs, the initial focus was on the "feel" of combat—the snappiness of the player’s movement, the weight of a sword swing, and the immediate feedback of a monster’s death animation. This led to a period of rapid, unbridled prototyping where speed of implementation was prioritized over architectural purity. This period, while productive in generating a playable vertical slice, inevitably led us into what we now refer to as the "Monolithic Trap."

### The God Object Pattern
The technical manifestation of this trap was centered around two primary classes: `AR1Player` and `AR1Monster`. At the time, these were quintessential "God Objects." In our haste to see the game in motion, we consolidated disparate systems into these single files. Input handling, attribute management, combat state machines, and even inventory logic were all hardcoded within the character classes. 

For instance, the player’s health and mana were not part of an extensible attribute system but were simple `float` variables defined directly in the header. Damage calculation was a sprawling switch statement within a `TakeDamage` override, manually checking for types, resistances, and active "buffs" that were tracked via an ever-growing list of boolean flags like `bIsStunned`, `bHasPowerBuff`, or `bIsInvulnerable`. The inventory was a simple `TArray` of `AActor` pointers, requiring the spawning of physical, hidden actors just to track an item’s existence in a player's backpack—a massive waste of memory and CPU cycles during level transitions.

This approach was deceptively efficient at first. If we needed a new mechanic, we simply added a new function or variable to `AR1Player.cpp`. There were no complex interfaces to navigate, no subsystems to register with, and no data assets to configure. The entire game logic lived in a few thousand lines of code that were easily accessible. However, as the scope of the project expanded from a single room to a multi-floor procedural dungeon with varying enemy types, the cracks in this foundation began to widen into chasms.

### The Cost of Tight Coupling
The most immediate cost of this tight coupling was the stifling of iteration and the introduction of "Fragile Code." As the classes grew, so did the compilation times and the cognitive load required to make even minor changes. Because every system was intertwined, a change to the input handling logic to support a new controller type could inadvertently break the damage calculation or cause a race condition in the inventory replication. 

The technical debt became most apparent during our first attempt at multiplayer integration. Because stats and combat logic were manually handled within the actors, ensuring consistent state across the network required a Herculean effort of RPC (Remote Procedure Call) management. We found ourselves writing redundant replication logic for every new variable, leading to a "code bloat" that was both difficult to debug and highly susceptible to desynchronization. If the server updated a player's health but forgot to trigger the corresponding `OnRep` function for the UI, the game state would drift, leading to the dreaded "ghost death" scenarios where a player would be dead on the server but still running on the client.

### The Developer's Perspective: The Wall
The true "moment of realization" came when we attempted to introduce a new monster type: the Ranger. Up until that point, our monsters were simple melee bruisers that shared the same basic `AR1Monster` logic. The Ranger required entirely new logic for ranged targeting, projectile spawning, and kiting behavior. 

In the monolithic architecture, we were faced with two equally poor choices: either add another thousand lines of conditional logic to the base `AR1Monster` class, or create a deep inheritance hierarchy (e.g., `AR1Monster` -> `AR1RangedMonster` -> `AR1Ranger`) that would inevitably lead to the "Fragile Base Class" problem. Attempting to implement the Ranger’s "Quick Shot" ability—which required a temporary attack speed buff—meant modifying the core damage functions to account for a new set of timers and state checks. 

It was during this week of development that we hit "The Wall." Adding a single feature was now taking three times longer than it had at the start of the project. We spent 80% of our time fixing regressions and only 20% on new content. Every bug fix felt like playing a game of "whack-a-mole"; squashing one bug in the AI logic would cause another to pop up in the animation blueprint or the sound system. The architectural debt had reached a point where the interest payments were consuming our entire development budget. 

### Transitioning to Scalability
It became clear that for Project R1 to survive and thrive as a production-grade RPG, a radical departure from inheritance-heavy, monolithic structures was mandatory. We needed a system that favored **composition over inheritance** and **data over hardcoded logic**. We realized that the "Character" class should be a thin shell—a container for components—rather than the brain of the entire game.

This led to the systematic dismantling of the God Objects. We identified that core gameplay logic—like stats, status effects, and damage math—belonged in a dedicated framework like the **Gameplay Ability System (GAS)**. Global management logic—like dungeon generation state and item databases—belonged in persistent **Subsystems** that could exist independently of any specific level or actor. We also realized that the "magic numbers" and hardcoded logic for monsters and items needed to be moved into **Primary Data Assets**, allowing designers to tune values like "Attack Range" or "Critical Hit Chance" without touching a single line of C++. 

The switch was not merely a "cleanup" of the code; it was a fundamental shift in our engineering philosophy. By moving to a decoupled, data-driven architecture, we weren't just fixing bugs; we were building a platform that could scale to accommodate the hundreds of unique items and enemy types that a modern ARPG demands. This transition, while challenging to execute mid-development, was the catalyst that allowed Project R1 to move from a fragile prototype to a robust, high-performance production environment.

## Phase 2: Procedural World & Memory Optimization

As Project R1 moved away from its monolithic roots, the next major architectural challenge was the world itself. In a modern Action RPG, the environment is not merely a static backdrop but a dynamic, evolving entity. We needed a system that could generate complex, non-linear dungeon layouts that felt hand-crafted yet remained entirely procedural. More importantly, we needed to ensure that this dynamic world did not come at the cost of performance. Loading a new room should not cause a "hitch" or a frame-drop, and the memory footprint had to remain stable regardless of the dungeon’s total size.

This phase of development focused on two critical systems: the **World Generation Algorithm** and the **Room Streaming Subsystem**. Together, they form the "heartbeat" of the R1 world, managing the physical layout and the underlying memory lifecycle of every corridor and combat arena.

### Part 1: The World Generation Algorithm

Most procedural generation in early-stage projects relies on simple random walks or grid-based noise. While these methods are easy to implement, they often produce layouts that feel "soulless"—long, winding corridors that lead nowhere, or clusters of rooms that overlap in nonsensical ways. For R1, we adopted a more structured approach: an **Isaac-style queue-based branching algorithm**.

#### The Logic of Branching
Managed by the `AR1MapGenerator` class, this algorithm views the dungeon as a mathematical graph rather than a simple grid. The generation begins at a central "Start Node" and uses a BFS (Breadth-First Search) inspired approach to "grow" the dungeon. Instead of placing rooms randomly, the generator maintains a queue of active nodes that are looking for neighbors. 

The core innovation lies in how we handle connectivity. Every room in R1 is defined by a `UR1RoomDefinitionData` Primary Data Asset. This asset doesn't just contain the level name; it contains the metadata for its "Available Doors" (North, South, East, West). During generation, the `AR1MapGenerator` doesn't just pick a random room; it performs a **puzzle-piece validation**. If the parent node has a "North" door, the generator specifically queries the room pool for a definition that contains a "South" door. This ensures that every generated connection is physically valid and that players never encounter a door that leads into a solid wall.

#### Preventing "The Blob"
A common pitfall in branching algorithms is the tendency for rooms to cluster together in a dense "blob," destroying the sense of exploration. To counter this, we implemented a **Grid Neighbor Check**. Before a new room is finalized at a specific grid coordinate, the generator checks its immediate neighbors. If a potential position already has more than one adjacent room, the generator discards the branch. This forces the dungeon to spread out, creating the branching paths and dead-ends (often leading to treasure or mini-bosses) that are characteristic of the genre's best level design.

```cpp
// Core Generation Loop Snippet from AR1MapGenerator.cpp
while (!RoomQueue.IsEmpty() && CurrentNodeID < TotalRoomCount)
{
    int32 ParentID;
    RoomQueue.Dequeue(ParentID);
    FR1MapNode& ParentNode = GeneratedMap[ParentID];

    // Shuffle door directions to ensure non-linear growth
    TArray<ER1DoorDirection> ParentDoors = ParentNode.RoomDefinition->AvailableDoors;
    for (int32 i = ParentDoors.Num() - 1; i > 0; i--) { ParentDoors.Swap(i, FMath::RandRange(0, i)); }

    for (ER1DoorDirection DoorDir : ParentDoors)
    {
        FIntPoint NewPos = ParentNode.GridPosition + GetDirOffset(DoorDir);
        
        // 1. Occupancy Check: Ensure the space is empty
        if (GetNodeIDAt(NewPos) != -1) continue;

        // 2. Cluster Prevention: Isaac-style neighbor check
        if (GetNeighborCount(NewPos) > 1) continue;

        // 3. Puzzle Piece Fit: Find a room with the matching door
        ER1DoorDirection OppositeDir = GetOppositeDir(DoorDir);
        UR1RoomDefinitionData* NextRoomData = PopValidRoomFromPool(CombatRoomPool, OppositeDir);

        if (NextRoomData)
        {
            // Finalize Node and Enqueue for further branching
            RegisterNewNode(CurrentNodeID, NewPos, NextRoomData, ParentID);
            RoomQueue.Enqueue(CurrentNodeID++);
        }
    }
}
```

By separating the **Generation Logic** (the graph growth) from the **Room Definitions** (the data assets), we created a system where designers can add entirely new room shapes or connectivity patterns without ever touching the C++ code. The generator simply treats the new `UR1RoomDefinitionData` as another piece of the puzzle.

### Part 2: The Room Streaming Subsystem

Once the map layout is generated, the next challenge is rendering it. In a large dungeon, spawning 50+ rooms simultaneously would lead to an immediate crash due to memory exhaustion. Conversely, loading rooms only when the player enters them causes significant "hitches" as the engine pauses to load assets.

The solution was the `UR1RoomStreamingSubsystem` and its **Thermal State Machine**.

#### The Thermal State Machine
To manage the lifecycle of a room, we categorize its "temperature" based on its proximity to the player. This state machine ensures that we are always one step ahead of the player's movement, loading data into memory before it's needed and unloading it once it's no longer relevant.

1.  **Cold:** The room exists only as a node in the map generator. No assets are in memory.
2.  **Preloading:** The player is two rooms away. The subsystem begins an asynchronous load of the room’s level and its core `PrimaryDataAssets` (textures, sounds, monster meshes).
3.  **Warm:** The level is loaded and the room is physically spawned in the world, but it is invisible and has no active AI or physics. It is "simmering," ready to be activated instantly.
4.  **Hot:** The player is in the room or an immediately adjacent room. The room is visible, AI is active, and all systems are running.

#### FR1RuntimeBudget: Engineering Stability
To prevent memory overflows, the subsystem operates under a strict `FR1RuntimeBudget`. This is a data-driven configuration that limits how many rooms can be "Warm" or "Hot" at any given time. If the player moves quickly through the dungeon, the subsystem doesn't just keep loading; it actively "trims" the oldest Warm rooms (those with the oldest `LastTouchedTime`) to stay within the memory budget.

This "Budget-Aware" streaming is critical for multi-platform compatibility. On a high-end PC, we might allow 10 Warm rooms for seamless backtracking; on a console or lower-spec machine, we can tune the `MaxPreloadedRooms` down to 3, trading a bit of backtracking speed for total system stability.

```cpp
// ER1RoomThermalState and Transition Logic from R1RoomStreamingSubsystem.h/cpp
UENUM(BlueprintType)
enum class ER1RoomThermalState : uint8
{
    Cold,       // On Disk
    Preloading, // Async Loading
    Warm,       // Loaded but Inactive
    Hot,        // Active & Visible
};

void UR1RoomStreamingSubsystem::TickRoomCachePolicy()
{
    const double Now = FPlatformTime::Seconds();
    for (auto& Pair : RoomStates)
    {
        FR1RoomRuntimeState& State = Pair.Value;
        if (State.ThermalState == ER1RoomThermalState::Cold) continue;

        // Hot rooms are protected from unloading
        if (State.ThermalState == ER1RoomThermalState::Hot) {
            State.LastTouchedTime = Now;
            continue;
        }

        // Graceful Unloading: Transition Warm -> Cold after timeout
        if (Now - State.LastTouchedTime > Budget.UnloadGraceSeconds)
        {
            UnloadRoomInternal(State); // Purge level and release asset handles
            State.ThermalState = ER1RoomThermalState::Cold;
        }
    }
}
```

The combination of the `AR1MapGenerator` and the `UR1RoomStreamingSubsystem` transformed the R1 world from a series of static levels into a living, breathing ecosystem. By treating the world as a data-driven graph and managing its memory via a state-aware subsystem, we achieved the "Holy Grail" of ARPG development: a seamless, non-linear, and infinitely expandable world that maintains a rock-solid 60 FPS on target hardware. This architecture doesn't just support the current game; it provides the foundation for any future expansion, allowing us to scale from small crypts to massive, sprawling underworlds with the change of a few variables in a Data Asset.

