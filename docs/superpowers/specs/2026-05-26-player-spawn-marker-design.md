# Design: Player Spawn Marker for New Run Start

**Date:** 2026-05-26  
**Branch:** feat-option-system-language (to be merged / branched from master)  
**Status:** Approved

---

## Problem

When a new run begins, `AR1MapGenerator::RegisterRoomManager()` always teleports the player to `GeneratedMap[0].SpawnLocation + FVector(0, 0, 150)`. Room 0's `SpawnLocation` is always `FVector::ZeroVector`, so the player always lands at world position `(0, 0, 150)` — the geometric centre of the start room's streamed sublevel — regardless of where the room's layout intends the player to stand.

The same fixed-centre fallback applies when `GoToNextFloor()` spawns the first room of a new floor.

---

## Goals

- Allow a level designer to place a marker Actor inside a start room sublevel that controls where the player spawns when entering that room for the first time (new run or new floor).
- Zero impact on the door-transition path or the save-load path.
- No new data assets, no changes to `UR1RoomDefinitionData`.

---

## Out of Scope

- Spawn markers for combat / boss / shop rooms (door-transition logic already handles those).
- Multiple spawn points with weighted selection.
- Any change to save/load behaviour.

---

## Architecture

### New Class: `AR1PlayerSpawnMarker`

**Files**
- `Source/R1/Map/R1PlayerSpawnMarker.h`
- `Source/R1/Map/R1PlayerSpawnMarker.cpp`

**Responsibility**  
A purely editor-time marker. It carries no runtime logic. Its world `Location` and `Rotation` are read once by `AR1MapGenerator` after the sublevel loads, then ignored.

**Members**

| Member | Type | Notes |
|---|---|---|
| `ArrowComponent` | `UArrowComponent*` | Shows player facing direction in the viewport |
| `BillboardComponent` | `UBillboardComponent*` | Editor sprite; hidden in-game |

**Class flags**
- `UCLASS(Blueprintable)` — allows a Blueprint child `BP_PlayerSpawnMarker` for drag-and-drop placement.
- `PrimaryActorTick.bCanEverTick = false`
- No collision, no mesh, no tick.

---

### Modified: `AR1MapGenerator::RegisterRoomManager()`

**File:** `Source/R1/Map/R1MapGenerator.cpp`

**Where:** Inside the `if (PendingNodeID == -1)` → `if (PlayerCharacter)` block, in the `else` branch that currently handles new-run / new-floor spawning (i.e. `!bIsLoadingFromSave`).

**Change:** Replace the single-line fallback with a marker lookup:

```cpp
// Before
FinalLocation = GeneratedMap[MatchedNodeID].SpawnLocation + FVector(0.0f, 0.0f, 150.0f);

// After
AActor* SpawnMarker = UGameplayStatics::GetActorOfClass(GetWorld(), AR1PlayerSpawnMarker::StaticClass());
if (SpawnMarker)
{
    FinalLocation = SpawnMarker->GetActorLocation();
    FinalRotation = SpawnMarker->GetActorRotation();
}
else
{
    FinalLocation = GeneratedMap[MatchedNodeID].SpawnLocation + FVector(0.0f, 0.0f, 150.0f);
}
```

`FinalRotation` is already declared in this block (currently `FRotator::ZeroRotator`), so reading the marker's rotation is a free addition.

---

## Fallback & Safety

| Scenario | Behaviour |
|---|---|
| Marker placed in sublevel | Player spawns at marker's world transform |
| No marker in sublevel | Existing fallback: room origin + `(0, 0, 150)` — no regression |
| Save-load path (`bIsLoadingFromSave == true`) | Unchanged — uses `LoadedPlayerLocation / LoadedPlayerRotation` |
| Door-transition path (`PendingNodeID != -1`) | Unchanged — uses door-position logic |
| Multiple markers present | `GetActorOfClass` returns first found; design intent is one per start room |

---

## Editor Workflow

1. Open the start room sublevel in the UE editor.
2. Place `BP_PlayerSpawnMarker` at the desired player spawn location.
3. Rotate it so the arrow faces the direction the player should look on spawn.
4. Save the sublevel.

No C++ rebuild is needed after placement. No data asset changes required.

---

## Files Affected

| File | Change |
|---|---|
| `Source/R1/Map/R1PlayerSpawnMarker.h` | **New** |
| `Source/R1/Map/R1PlayerSpawnMarker.cpp` | **New** |
| `Source/R1/Map/R1MapGenerator.h` | Add `#include` forward declaration |
| `Source/R1/Map/R1MapGenerator.cpp` | Replace hardcoded fallback with marker lookup (≈8 lines) |

---

## Testing Checklist

- [ ] New run: player spawns at marker position and faces marker rotation.
- [ ] New run without marker: player spawns at room centre + Z150 (no regression).
- [ ] Load from save: player spawns at saved location (no regression).
- [ ] Door transition: player spawns near door entry (no regression).
- [ ] `GoToNextFloor()`: player spawns at new floor's start room marker (or centre fallback if absent).
- [ ] Multiple markers in level: no crash; player spawns at one of them.
