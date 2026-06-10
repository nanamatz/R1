# Floor Asset Preload During Loading — Design

**Date:** 2026-06-10
**Status:** Approved (pending spec review)
**Area:** `AR1MapGenerator` loading flow / `UR1AssetData` / `UR1AssetManager`

## Problem

The pre-gameplay loading sequence in `AR1MapGenerator` front-loads map data, room
geometry (streamed levels), navmesh, input gating, and the camera fade. It does **not**
front-load the heavy runtime content referenced by a floor's content — monster meshes /
anim blueprints, theme VFX/SFX, and drop assets. These are soft-referenced and load
just-in-time on first spawn / first cast, producing a hitch on first combat.

`UR1RoomDefinitionData` already declares the data fields intended to fix this:

```cpp
// 룸 진입 전에 함께 로드할 Primary Assets (몬스터 아키타입, 드랍, 테마 VFX/SFX 등)
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
TArray<FPrimaryAssetId> PreloadPrimaryAssets;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
TArray<FName> PreloadAssetLabels;
```

A codebase search confirms **nothing reads these fields** — they are dead. The feature
was specified in data but never wired up.

Note: `AR1MonsterSpawner::MonsterClass` is a hard `TSubclassOf` reference, so the monster
*class* is already loaded with the room level. The value of this feature is forcing the
soft-referenced sub-assets (meshes/anim/VFX/SFX) and designer-curated theme/drop assets
to be resident before gameplay begins.

## Goals

- Consume the existing `PreloadPrimaryAssets` / `PreloadAssetLabels` fields.
- Async-load a floor's combined preload set during the loading screen, overlapping the
  existing slow level-streaming work.
- Hold the loading screen until the preload completes, with a safety timeout so a missing
  or bad asset id can never hang loading forever.
- Keep preloaded assets resident for the lifetime of the floor; release them on floor
  transition so memory is bounded to ~one floor's preload set.

## Non-Goals

- Object-pool pre-warming (separate task, descoped).
- PSO / shader / Niagara warm-up (separate task, descoped).
- Auto-discovery of assets from room actors. The asset list is **designer-populated only**;
  empty fields are a safe no-op for that room.
- Any change to replication / RPC (project is single-player; do not add network branches).
- Any change to `R1LoadingSubSystem` gate semantics (`bSceneDone` / `bContentReady`).

## Design

### Data source

Per-room PDA fields `PreloadPrimaryAssets` (`TArray<FPrimaryAssetId>`) and
`PreloadAssetLabels` (`TArray<FName>`). Designers populate them in-editor per room.
Empty fields → no extra assets loaded for that room.

### Integration points — `AR1MapGenerator`

All changes are contained in `AR1MapGenerator`; `R1LoadingSubSystem` is untouched.

**1. Kick off the preload (overlaps with level streaming).**
At the top of `SpawnFloorAndWait()` — which is reached by both the fresh-generate path
(`InitializeMap`) and the save-restore path (`LoadMapFromSaveData`) — after the floor's
rooms are known, gather the de-duplicated preload set from every
`GeneratedMap[i].RoomDefinition`:

- `PreloadAssetLabels` → resolved to `FSoftObjectPath`s via
  `UR1AssetData::GetAssetSetByLabel(Label)` (the loaded global asset data,
  `UR1AssetManager::GetLoadedAssetData()`).
- `PreloadPrimaryAssets` → resolved to `FSoftObjectPath`s via the asset manager
  (`UAssetManager::GetPrimaryAssetPath` / equivalent), including any associated bundles
  needed.

Merge all unique paths into a **single** `UAssetManager::GetStreamableManager().RequestAsyncLoad(...)`
call. Store the returned `TSharedPtr<FStreamableHandle>` in a new member
`FloorPreloadHandle`. Starting the request here lets disk I/O run in parallel with the
slow `AddToWorld` (`OnLevelShown`) level streaming — no added wall-clock in the common case.

If the merged path set is empty, leave `FloorPreloadHandle` null (gate treats null as
"already done").

**2. Gate on completion (reuses the existing poll loop).**
`WaitForNavMeshThenActivate()` already polls on `NavBuildWaitTimer` until the navmesh is
ready before calling `NotifyContentReady()` + `ActivateRoom()`. Add one condition to that
same gate, evaluated alongside the navmesh check:

```
preloadReady = (FloorPreloadHandle == nullptr) || FloorPreloadHandle->HasLoadCompleted();
```

If the navmesh is ready but `preloadReady` is false, keep polling on the existing timer.
Only when **both** navmesh and preload are ready (or the timeout fires) does it proceed to
`NotifyContentReady()` + `ActivateRoom()`. No new timer or member counter is introduced
beyond `FloorPreloadHandle`.

**3. Timeout fallback.**
Reuse the existing timeout window (`NavBuildWaitTicks >= NavBuildMaxTicks`, ~5s). On
timeout, if the preload handle is still incomplete, log a warning that names the
unfinished preload, then proceed anyway. Loading never hangs. This mirrors the existing
navmesh timeout philosophy.

### Lifetime / release

`FloorPreloadHandle` is a plain member:

```cpp
TSharedPtr<FStreamableHandle> FloorPreloadHandle;
```

It is **not** a `UPROPERTY` (a streamable handle is not a UObject). Holding the handle
keeps the floor's preloaded assets resident. Before starting the next floor's request in
`GoToNextFloor()` (near the existing `CleanupFloorActors()` / `UnloadAllRooms()` cleanup),
reset the previous handle so the prior floor's preloaded assets become GC-eligible. This
bounds resident preload memory to ~one floor.

## Error Handling

| Condition | Behavior |
|-----------|----------|
| Room has empty preload fields | No-op for that room. |
| Floor preload set empty overall | `FloorPreloadHandle` null; gate treats as done immediately. |
| Async load slow | Gate holds the loading screen until complete. |
| Async load exceeds timeout | Warn (naming unfinished preload) and proceed; never hang. |
| Invalid/missing label or primary id | Skipped during path resolution with a warning; does not abort the batch. |

## Testing / Verification

Single-player editor verification (no automated harness for this flow):

1. Populate a combat room PDA's `PreloadAssetLabels` / `PreloadPrimaryAssets` with its
   monster/VFX assets. Enter the floor and confirm via log that the preload request is
   issued at floor-spawn start and completes before `NotifyContentReady()`.
2. Confirm first-combat in that room no longer hitches on first monster spawn / first cast
   (compare against an unpopulated room).
3. Leave a room PDA's fields empty — confirm loading still completes normally (no-op path).
4. Point a field at a deliberately invalid id — confirm a warning is logged and loading
   still completes (timeout/skip path, no hang).
5. Transition floors twice — confirm the previous floor's handle is reset (memory does not
   grow unbounded across floors).

## Files Touched

- `Source/R1/Map/R1MapGenerator.h` — add `TSharedPtr<FStreamableHandle> FloorPreloadHandle;`
  member and a private helper declaration (e.g. `StartFloorAssetPreload()`).
- `Source/R1/Map/R1MapGenerator.cpp` — gather/kick off preload in `SpawnFloorAndWait`,
  extend the gate in `WaitForNavMeshThenActivate`, reset handle in `GoToNextFloor`.

No changes to `R1LoadingSubSystem`, `R1RoomStreamingSubsystem`, or the room PDA struct
(fields already exist).

## Build Note

`R1MapGenerator.h` gains a new member, so this requires a full VS2022 build (not Live
Coding) per the project's header-change rule.
