# Design Spec: Player Location Persistence

This document outlines the changes required to save and restore the player's exact world location and rotation in the R1 project.

## 1. Problem Statement
Currently, when a player loads a saved game, they are teleported to the center of the room (`SpawnLocation` of the active node). This breaks immersion and can be disorienting if the player saved while in a specific spot.

## 2. Proposed Solution
We will store the player's absolute world location and rotation in the `UR1PlayerSaveGame` object and ensure the `AR1MapGenerator` uses these values instead of default room-center coordinates when a save file is loaded.

## 3. Technical Changes

### 3.1 Data Structures
Update `UR1PlayerSaveGame` in `R1PlayerSaveGame.h`:
- Add `UPROPERTY() FVector PlayerLocation`.
- Add `UPROPERTY() FRotator PlayerRotation`.

### 3.2 Save System
Update `UR1SaveSystem::SaveCurrentRun` in `R1SaveSystem.cpp`:
- Capture `Player->GetActorLocation()` and `Player->GetActorRotation()` and store them in the `SaveObj`.

### 3.3 Map Generator
Update `AR1MapGenerator` in `R1MapGenerator.h` and `R1MapGenerator.cpp`:
- Add members to track loaded spatial data:
  - `bool bIsLoadingFromSave = false;`
  - `FVector LoadedPlayerLocation;`
  - `FRotator LoadedPlayerRotation;`
- Update `LoadMapFromSaveData` to store the incoming location/rotation from the save object.
- Update `RegisterRoomManager`:
  - When `PendingNodeID == -1` (initial load logic), check if `bIsLoadingFromSave` is true.
  - If true, teleport the player to `LoadedPlayerLocation` with `LoadedPlayerRotation`.
  - Reset `bIsLoadingFromSave` to false after the first teleport to prevent it from affecting subsequent room transitions.

### 3.4 Save System - Loading
Update `UR1SaveSystem::LoadCurrentRun` in `R1SaveSystem.cpp`:
- Pass the loaded `PlayerLocation` and `PlayerRotation` to `AR1MapGenerator::LoadMapFromSaveData`.

## 4. Verification Plan
- **Pre-test**: Stand at a specific recognizable spot (e.g., near a wall) and trigger an auto-save (or room transition which triggers one).
- **Test**: Quit to main menu and reload.
- **Expected Result**: Player should be at the exact same world location and facing the same direction.
- **Regression**: Move between rooms normally to ensure standard room transition teleportation (to doors) still works correctly.
