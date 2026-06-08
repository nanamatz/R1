# LoL-style Smooth Movement (Player) — Design

**Date:** 2026-06-08
**Branch:** feat-HP-text-UI (work to be done here unless a dedicated branch is created)
**Scope:** Player character only. Single-player UE5.3 / GAS project.

## Problem

Player click-to-move follows the Recast navmesh path, but the motion looks jagged: the
character cuts across polygon corners and changes facing abruptly at each path point.
The goal is LoL-style perceived smoothness without replacing the navmesh with a grid.

Three independent stages contribute to smoothness; the project currently only has Stage 1
(stock navmesh) and is missing Stages 2 and 3:

1. **Path** — navmesh resolution (editor/config side).
2. **Smoothing** — string-pulling the path points in code (currently absent).
3. **Movement** — rotation/acceleration interpolation on CharacterMovement (currently a
   single hardcoded `RotationRate`, no acceleration tuning).

## Current State (verified)

- `R1PlayerController.cpp` issues all player movement via
  `UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Dest)` at three call sites:
  - `OnSetDestinationTriggered()` (mouse-drag move, gated by a 50-unit destination delta)
  - `OnSetDestinationReleased()` (short-press click move)
  - `ChaseTargetAndAttack()` (chase target / interactable)
  There is **no access to raw path points**, so no string-pulling happens beyond Recast's
  built-in funnel.
- `R1Player.cpp` ctor: `bOrientRotationToMovement = true`,
  `RotationRate = FRotator(0, 640, 0)`. No `MaxAcceleration` / braking / friction tuning.
- AI monsters move via a separate path (`BTTask_MoveToRange`, `R1AIController::MoveTo`) —
  **out of scope** (player-only by decision).
- `R1.Build.cs` already lists `NavigationSystem` and `AIModule` — no module change needed.

## Decisions

- **Target:** Player only. AI monster paths are untouched.
- **Stage 2 approach:** Custom path query + greedy NavRaycast string-pull (not just tuning
  Recast/PathFollowing).
- **Stage 3 depth:** Config-only ctor tuning (no per-frame Tick logic). CharacterMovement
  already interpolates rotation toward velocity each frame; with the straightened Stage-2
  path this reads as smooth. A per-frame RotationRate-easing variant was explicitly
  deferred (YAGNI unless config-only looks stiff at sharp turns).
- **Stage 1 (navmesh resolution):** Editor/Project-Settings side. Out of code scope; values
  documented below as guidance for the user to apply in-editor.

## Architecture

A new isolated unit owns Stage 2 so the controller stays thin and the smoothing is testable
in isolation.

### `UR1NavSmoothingLibrary` (new) — `Source/R1/Library/R1NavSmoothingLibrary.{h,cpp}`

A `UBlueprintFunctionLibrary` with a single public entry point:

```cpp
UFUNCTION(BlueprintCallable, Category = "Navigation")
static void SmoothMoveTo(AController* Controller, const FVector& Destination);
```

**Responsibility:** given a controller + world destination, produce a string-pulled path and
issue the move. Knows nothing about UI, combat, or controller state.

**Flow:**

1. **Validate inputs** — `Controller`, its `APawn`, `UWorld`, and
   `UNavigationSystemV1::GetCurrent(World)`. Any null → return (no crash).
2. **Query path** — build `FPathFindingQuery` from pawn location → `Destination` using the
   pawn's nav agent properties and the default nav data; call
   `NavSys->FindPathSync(Query)`.
3. **Guard / fallback** — if `Result.IsSuccessful()` is false or the path has fewer than 3
   points (nothing to smooth), call
   `UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, Destination)` and return.
   Movement therefore never regresses below current behavior.
4. **String-pull (core)** — greedy pass over `Path->GetPathPoints()`:
   - Start with `Anchor = points[0]`, always keep it.
   - Probe the farthest subsequent point `P` for which
     `NavSys->NavigationRaycast(Controller, Anchor, P)` reports **no** navmesh hit
     (i.e. the straight line stays on the navmesh).
   - Keep that farthest reachable point as the new `Anchor`; discard the skipped midpoints.
   - Repeat until the destination point is the anchor.
   - Always retain the final destination point.
   This collapses polygon-corner zig-zags that the corridor only visited because of polygon
   shape, while remaining guaranteed-on-navmesh (so it cannot path through walls).
5. **Issue move** — overwrite the path's point list with the kept points, get-or-create the
   controller's `UPathFollowingComponent` (mirroring the init `SimpleMoveToLocation` does
   internally), and call
   `PFollowComp->RequestMove(FAIMoveRequest(Destination).SetUsePathfinding(false), Path)`.
   `SetUsePathfinding(false)` is used because we already supply a finished path.

**Dependencies:** `NavigationSystem` (FindPathSync, NavigationRaycast), `AIModule`
(PathFollowingComponent, FAIMoveRequest, UAIBlueprintHelperLibrary). Both already linked.

### `R1PlayerController` (modified) — `Source/R1/Player/R1PlayerController.cpp`

Replace the move call at the three sites only:

- `SimpleMoveToLocation(this, CacheDestination)` → `UR1NavSmoothingLibrary::SmoothMoveTo(this, CacheDestination)`
- in `ChaseTargetAndAttack`, both target and interactable branches likewise.

No other controller logic changes. The existing 50-unit destination gate in
`OnSetDestinationTriggered` continues to throttle re-queries, so per-tick `FindPathSync`
cost is bounded (and is comparable to what `SimpleMoveToLocation` already did).
Add `#include "Library/R1NavSmoothingLibrary.h"` to the controller .cpp.

### `R1Player` (modified) — `Source/R1/Character/R1Player.{h,cpp}` ctor

Replace the two hardcoded movement lines with designer-tunable `UPROPERTY` fields and set
smooth defaults. Proposed defaults (all `EditAnywhere`, tunable in BP):

- `bOrientRotationToMovement = true` (kept)
- `RotationRate = FRotator(0, 720, 0)` (snappy but smooth turn)
- `MaxAcceleration ≈ 2048`
- `BrakingDecelerationWalking ≈ 2048`
- `bUseSeparateBrakingFriction = true`, `BrakingFriction ≈ 8`

These are CharacterMovement configuration, not new gameplay branching, and comply with the
single-player "no new replication/RPC/authority branches" rule.

## Stage 1 guidance (editor — user applies)

Not code. On the level's `RecastNavMesh` actor (or Project Settings → Navigation Mesh):
- Lower **Cell Size** and **Cell Height** (finer voxels → tighter corridors). Start by
  halving current values; rebuild navmesh and compare cost.
- Optionally raise **Tile Size UU** resolution as needed.
Higher resolution alone only shrinks polygons; the smoothness comes from Stages 2 + 3.

## Error Handling

- All null-checks in `SmoothMoveTo` fail safe by returning or falling back to
  `SimpleMoveToLocation`.
- A failed/short path falls back to stock behavior — never worse than today.
- String-pull only keeps points proven on-navmesh by raycast → cannot cut through geometry.

## Testing / Verification

Manual in-PIE (this is movement feel; no unit-test harness for navmesh in the project):
1. Click far across an L-shaped corridor: character should take a visibly straighter,
   corner-cutting-but-on-navmesh path vs. the current stair-step.
2. Click directly in line of sight: behaves identically to before (fallback path).
3. Drag-move (hold): no stutter; 50-unit gate still throttles.
4. Chase a monster around a corner, then into attack range: smooth approach, clean stop
   (verify `StopMovement` / attack range logic in `ChaseTargetAndAttack` still fires).
5. Confirm no regression in `ResetMovementState` (teleport/abort) behavior.

## Out of Scope

- AI monster path smoothing.
- Per-frame RotationRate easing (deferred Stage-3 variant).
- Any navmesh asset edits (editor-side, user-applied).
- Multiplayer/replication — project is single-player; no net code added.
