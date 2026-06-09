# Floor-at-Once Loading — Design

**Date:** 2026-06-09
**Branch:** refactor-asset-preloading-system
**Status:** Approved (design); pending implementation plan

## Problem

Entering a room for the first time hitches. The current per-room lazy
streaming async-preloads adjacent room *packages*, but crossing a door still
runs `ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr` → `AddToWorld`
(actor spawning, component registration, BeginPlay, collision/nav). `AddToWorld`
is the actual hitch, and preloading the package does not avoid it.

## Goal

Load an entire floor's room sublevels up front, under the loading screen, so all
`AddToWorld` cost is paid at a moment the player expects to wait. In-play door
transitions become pure teleports (zero streaming hitch). Floor transitions
(loading screen → unload floor → load next floor) keep the same model.

Floors stay small (~7–12 rooms), so whole-floor residency is safe and the
thermal/budget streaming machinery can be retired.

## Principle

**Decouple room *load* from room *activation*.** Today a room's streaming-load
callback (`DungeonManager::BeginPlay` → `RegisterRoomManager`) is what spawns
monsters, locks doors, and teleports the player. Loading all rooms at once would
fire every room's activation simultaneously. The fix is to make loading passive
and activation explicit on entry.

## Components

### 1. `UR1RoomStreamingSubsystem` — slim down

- **Keep:** `SpawnRoomLevel` (still dedups via a slim `RoomStates`), `UnloadAllRooms`.
- **Delete (private, not Blueprint-exposed — safe):** `BeginPreload`,
  `TrimPreloadIfNeeded`, the thermal fields, `PreloadHandle`, `Budget`, and the
  thermal branches in `UnloadRoomInternal`. `FR1RoomRuntimeState` reduces to
  `RoomDefinition` + `StreamingLevel`.
- **Convert to deprecated no-op stubs (Blueprint-exposed — cannot verify BP
  callers from C++):** `QueuePreloadRooms`, `MarkRoomGameplayReady`,
  `MarkRoomAsLeft`, `TickRoomCachePolicy`, `SetRuntimeBudget`, `GetRuntimeBudget`,
  `GetRoomState`, and the `OnRoomBecameHot` delegate. Empty bodies, `UE_DEPRECATED`.
  (`TickRoomCachePolicy` has zero C++ callers, so a Blueprint tick is plausible.)

### 2. `AR1MapGenerator` — orchestration split

**Split `RegisterRoomManager`:**

- `RegisterRoomManager(Manager, NodeID)` — **passive**, runs on each room's
  `BeginPlay`: node match, `ActiveManagers.Add`, `InitializeRoomData`, set up
  *all* door connections + bind `OnDoorEntered` / `OnRoomCleared`, apply
  cleared-room door visuals. No combat, no teleport.
- `ActivateRoom(int32 NodeID)` — **explicit, on entry**: if not cleared →
  `LockRoomDoors` + `StartRoomCombat` (treasure → auto-complete); resolve teleport
  target (door-relative / marker / saved location) + teleport; minimap update +
  autosave; record in `InitializedNodeIDs`.

**New floor-load driver:**

- `SpawnFloorAndWait()` — after `GenerateMap`, loop **all** nodes →
  `SpawnRoomLevel`, collect the `ULevelStreamingDynamic*`s, bind each
  `OnLevelLoaded` to a counter. Rooms already `IsLevelLoaded` are counted
  synchronously; spawn failures (null level) are counted too, so the gate can
  never stall. At N/N → `OnFloorFullyLoaded()`.
- `OnFloorFullyLoaded()` — progress → 1.0; `ActivateRoom(start or resumed node)`;
  notify the loading subsystem that content is ready.

**Call-site changes:**

- `InitializeMap`, `GoToNextFloor`, `LoadMapFromSaveData` all route through
  `SpawnFloorAndWait` → activate start (or resumed) node. `GoToNextFloor` keeps
  `UnloadAllRooms` first.
- `OnPlayerEnteredDoor` — room already resident → set pending direction →
  `ActivateRoom(NextNodeID)` directly. No `SpawnRoomLevel`, no `OnLevelLoaded`
  hookup, no `OnTransitionRoomLoaded`.
- Remove adjacent-preload blocks and `MarkRoomGameplayReady` / `MarkRoomAsLeft`
  calls. `OnTransitionRoomLoaded` / `OnSavedRoomLoaded` collapse into the
  all-loaded → activate path.

### 3. Loading-screen gating — `UR1LoadingSubSystem`

Today the screen hides via `LoadingWidget->OnSceneFinished` → 0.7s →
`HideLoadingScreen`, i.e. the widget animation timeline, independent of whether
rooms finished `AddToWorld`. With 7–12 rooms the animation can finish first and
the screen lifts mid-stream.

Hide only when **both** hold: widget `OnSceneFinished` **and** content-ready. Add
`bSceneDone` / `bContentReady` flags + a `NotifyContentReady()` the generator
calls in `OnFloorFullyLoaded`; `HideLoadingScreen` fires when both are true.

### 4. Marker scoping

Replace `GetActorOfClass(World, AR1PlayerSpawnMarker)` (returns the first marker
in the whole world — ambiguous with all rooms resident) with the marker
**nearest to `GeneratedMap[NodeID].SpawnLocation`** (robust given 5000-unit
spacing). Used only on new-floor start; save-resume keeps `LoadedPlayerLocation`.

## Data flow

```
Floor start
  → loading screen up
  → generate graph
  → spawn ALL sublevels  (each AddToWorld → DungeonManager::BeginPlay
                          → passive RegisterRoomManager)
  → all-loaded → OnFloorFullyLoaded
      → ActivateRoom(start/resume) + NotifyContentReady
  → screen hides when (scene finished AND content ready)
  → play

Door cross → ActivateRoom(target) instantly (teleport + combat) → autosave.
             No streaming.

Floor clear → portal → GoToNextFloor → loading screen → UnloadAllRooms → repeat.
```

## Edge cases

- Already-loaded and failed spawns counted, so the gate never hangs.
- Re-entering a cleared room: passive register already unlocked doors;
  `ActivateRoom` sees `bIsCleared` and skips combat (no monster re-spawn).
- BP stubs are no-ops, so existing Blueprint graphs still compile.
- Unentered rooms are resident but combat-dormant — monsters don't exist until
  `StartRoomCombat`. Doors only tick while opening; spawner/manager tick is off.

## Verified safe (no side effect)

- Streaming subsystem is consumed only by `R1MapGenerator.cpp`; deleting the
  thermal API breaks no other C++ file.
- `AR1MonsterSpawner::DungeonManager` is `EditInstanceOnly` (per-room wired) and
  `StartRoomCombat` filters `It->DungeonManager == this`; UE remaps intra-level
  references per streamed instance, so each manager spawns only its own monsters.
- HUD, minimap, `R1PlayerState`, `R1Portal` use `GetActorOfClass(AR1MapGenerator)`
  — single generator, unaffected.
- Door default `bLocked=false` is harmless for unentered rooms.

## Constraints (project)

- Single-player only — no new replication/RPC, no new `HasAuthority()` branches.
- Header changes (new `UFUNCTION`s, the `RegisterRoomManager`/`ActivateRoom`
  split, subsystem member removals) require a full VS2022 build with the editor
  closed; Live Coding will not pick them up.
- Do not edit binary `.uasset` / `.umap`.

## Testing (in-editor, single-player)

- First-floor door crossings are hitch-free.
- Only the entered room's monsters spawn.
- Loading screen holds until all rooms are loaded.
- Portal floor-transition unloads/reloads cleanly — no leftover monsters, no
  double-teleport.
- Save/resume mid-floor in a non-start room spawns at the right spot with only
  that room active.
