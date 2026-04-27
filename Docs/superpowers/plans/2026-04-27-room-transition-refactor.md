# Room Transition System Refactoring Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix the redundant `RegisterRoomManager` calls that cause `NavData RegistrationFailed` and refactor the room transition logic to be robust and efficient.

**Architecture:** We will implement a "Transition Guard" within `AR1MapGenerator` to prevent duplicate initialization during level streaming. We'll also refactor `RegisterRoomManager` to accept the `NodeID` as a parameter from the caller, eliminating the fragile location-based search and preventing the logic from incorrectly defaulting to Room 0.

**Tech Stack:** C++, Unreal Engine 5 (Level Streaming)

---

### Task 1: Refactor `RegisterRoomManager` Interface

**Files:**
- Modify: `Source/R1/Map/R1MapGenerator.h`
- Modify: `Source/R1/Map/R1MapGenerator.cpp`
- Modify: `Source/R1/Map/DungeonManager.cpp`

- [ ] **Step 1: Update Header Declaration**
Modify `AR1MapGenerator.h` to change `RegisterRoomManager` signature.
```cpp
// Change from:
void RegisterRoomManager(class ADungeonManager* Manager);
// To:
void RegisterRoomManager(class ADungeonManager* Manager, int32 RoomNodeID = -1);
```

- [ ] **Step 2: Update `DungeonManager.cpp`**
Pass `-1` (default) from `ADungeonManager::BeginPlay`.
```cpp
// Source/R1/Map/DungeonManager.cpp
void ADungeonManager::BeginPlay() {
    // ... existing logic to find Generator ...
    Generator->RegisterRoomManager(this, -1); 
}
```

- [ ] **Step 3: Refactor `AR1MapGenerator.cpp` implementation**
Update `RegisterRoomManager` to use the passed ID if valid, or use a cached lookup map if necessary. However, the most critical part is handling the `PendingNodeID` transition state properly to avoid the "Double Call" bug.

- [ ] **Step 4: Commit**
```bash
git add Source/R1/Map/R1MapGenerator.h Source/R1/Map/R1MapGenerator.cpp Source/R1/Map/DungeonManager.cpp
git commit -m "refactor: update RegisterRoomManager signature to support explicit NodeID"
```

---

### Task 2: Implement Transition Guard and Fix NavData Bug

**Files:**
- Modify: `Source/R1/Map/R1MapGenerator.h`
- Modify: `Source/R1/Map/R1MapGenerator.cpp`

- [ ] **Step 1: Add State Tracking**
Add a set or map to `AR1MapGenerator.h` to track which NodeIDs have already been initialized in the current session.
```cpp
UPROPERTY()
TSet<int32> InitializedNodeIDs;
```

- [ ] **Step 2: Update `RegisterRoomManager` Logic**
Modify the logic to check if a room is already registered and handle the `PendingNodeID` correctly.
```cpp
// Source/R1/Map/R1MapGenerator.cpp
void AR1MapGenerator::RegisterRoomManager(ADungeonManager* Manager, int32 RoomNodeID) {
    if (!IsValid(Manager)) return;

    // Use passed ID or find by location
    int32 MatchedNodeID = (RoomNodeID != -1) ? RoomNodeID : FindNodeIDByLocation(Manager->GetActorLocation());
    
    // GUARD: If this node is already initialized and we aren't explicitly transitioning to it, skip.
    if (InitializedNodeIDs.Contains(MatchedNodeID) && MatchedNodeID != PendingNodeID) return;

    // ... (rest of the registration logic) ...
    
    InitializedNodeIDs.Add(MatchedNodeID);
}
```

- [ ] **Step 3: Fix the "Room 0" defaulting bug**
Ensure `MarkRoomGameplayReady` uses `MatchedNodeID` instead of hardcoded `0`.

- [ ] **Step 4: Commit**
```bash
git add Source/R1/Map/R1MapGenerator.h Source/R1/Map/R1MapGenerator.cpp
git commit -m "fix: implement transition guard to prevent redundant registration and NavData errors"
```

---

### Task 3: Optimization of Level Streaming Callbacks

**Files:**
- Modify: `Source/R1/Map/R1MapGenerator.cpp`

- [ ] **Step 1: Consolidate `OnTransitionRoomLoaded` and `RegisterRoomManager`**
Streamline the callback process so that we don't rely on `TActorIterator` which is slow. Instead, have the `DungeonManager` "check-in" once the level is visible.

- [ ] **Step 2: Commit**
```bash
git add Source/R1/Map/R1MapGenerator.cpp
git commit -m "perf: optimize room registration by reducing TActorIterator usage"
```
