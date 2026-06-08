# LoL-style Smooth Movement (Player) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make player click-to-move read as LoL-smooth by string-pulling the navmesh path in code (Stage 2) and tuning CharacterMovement rotation/acceleration (Stage 3), player-only.

**Architecture:** A new `UR1NavSmoothingLibrary` (Blueprint function library) queries the navmesh, greedily collapses path points that have a clear on-navmesh straight line (NavRaycast string-pull), and issues the move through the controller's PathFollowingComponent. `R1PlayerController`'s three existing move calls switch to it. `R1Player`'s ctor gains tunable rotation/acceleration defaults.

**Tech Stack:** Unreal Engine 5.3, C++/MSVC, GAS project. Modules `NavigationSystem` + `AIModule` (already linked). No new replication/RPC (single-player rule).

---

## Important build / workflow notes

- This plan **adds a `UCLASS` and new `UPROPERTY` members** → Live Coding will NOT pick it up. After code changes: **close the editor, full-build in VS2022, relaunch.**
- Build command (adjust `<UE>` to your install; typical UE 5.3 path shown):

  ```bat
  "C:\Program Files\Epic Games\UE_5.3\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "C:\Users\owner\Documents\GitHub\R1\R1.uproject" -waitmutex
  ```

- There is **no automated test harness** for navmesh/movement in this project. Verification = successful compile + manual in-PIE observation. Each task states exactly what to look for.
- Branch: work on `feat-Improving-Navigation-System` (current).

---

## File Structure

- **Create** `Source/R1/Library/R1NavSmoothingLibrary.h` — declares `UR1NavSmoothingLibrary` with one static `SmoothMoveTo`.
- **Create** `Source/R1/Library/R1NavSmoothingLibrary.cpp` — path query + string-pull + RequestMove.
- **Modify** `Source/R1/Player/R1PlayerController.cpp` — swap 3 `SimpleMoveToLocation` calls; add include.
- **Modify** `Source/R1/Character/R1Player.h` — add tunable movement `UPROPERTY` fields.
- **Modify** `Source/R1/Character/R1Player.cpp` — ctor applies the fields.

---

## Task 1: Create `UR1NavSmoothingLibrary` (Stage 2 core)

**Files:**
- Create: `Source/R1/Library/R1NavSmoothingLibrary.h`
- Create: `Source/R1/Library/R1NavSmoothingLibrary.cpp`

- [ ] **Step 1: Create the header**

`Source/R1/Library/R1NavSmoothingLibrary.h`:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "R1NavSmoothingLibrary.generated.h"

/**
 * Player movement helper: queries the navmesh, string-pulls the path with NavRaycast
 * (drops midpoints that have a clear on-navmesh straight line), and issues the move.
 * Falls back to stock SimpleMoveToLocation when there is nothing to smooth or any input is invalid.
 */
UCLASS()
class R1_API UR1NavSmoothingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	static void SmoothMoveTo(AController* Controller, const FVector& Destination);
};
```

- [ ] **Step 2: Create the implementation**

`Source/R1/Library/R1NavSmoothingLibrary.cpp`:

```cpp
#include "Library/R1NavSmoothingLibrary.h"

#include "NavigationSystem.h"
#include "NavigationData.h"
#include "NavigationPath.h"
#include "AI/Navigation/NavigationTypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

void UR1NavSmoothingLibrary::SmoothMoveTo(AController* Controller, const FVector& Destination)
{
	// 1. Validate inputs — fail safe (no crash, no movement) on anything missing.
	if (Controller == nullptr)
	{
		return;
	}

	APawn* Pawn = Controller->GetPawn();
	UWorld* World = Controller->GetWorld();
	if (Pawn == nullptr || World == nullptr)
	{
		return;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (NavSys == nullptr)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, Destination);
		return;
	}

	const FVector Start = Pawn->GetNavAgentLocation();
	const ANavigationData* NavData = NavSys->GetNavDataForProps(Pawn->GetNavAgentPropertiesRef(), Start);
	if (NavData == nullptr)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, Destination);
		return;
	}

	// 2. Query the path synchronously.
	FPathFindingQuery Query(Pawn, *NavData, Start, Destination);
	FPathFindingResult Result = NavSys->FindPathSync(Query);

	// 3. Guard / fallback — invalid or too short to smooth (straight shot).
	if (!Result.IsSuccessful() || !Result.Path.IsValid() || Result.Path->GetPathPoints().Num() < 3)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, Destination);
		return;
	}

	// 4. String-pull: greedy — from each anchor keep the farthest point reachable by a
	//    clear on-navmesh straight line, dropping the midpoints in between.
	TArray<FNavPathPoint>& Pts = Result.Path->GetPathPoints();
	TArray<FNavPathPoint> Kept;
	Kept.Reserve(Pts.Num());
	Kept.Add(Pts[0]);

	int32 Anchor = 0;
	while (Anchor < Pts.Num() - 1)
	{
		int32 Farthest = Anchor + 1; // the immediate next point is always reachable
		for (int32 i = Anchor + 2; i < Pts.Num(); ++i)
		{
			FVector HitLocation;
			const bool bBlocked = UNavigationSystemV1::NavigationRaycast(
				Controller, Pts[Anchor].Location, Pts[i].Location, HitLocation, nullptr, Controller);

			if (bBlocked)
			{
				break; // can't see point i directly — stop extending from this anchor
			}
			Farthest = i; // clear straight line on navmesh — we can go straight to i
		}

		Kept.Add(Pts[Farthest]);
		Anchor = Farthest;
	}

	Pts = MoveTemp(Kept); // overwrite the path with the smoothed point list

	// 5. Issue the move through a PathFollowingComponent (get-or-create, mirroring
	//    what SimpleMoveToLocation does internally). Path already finished → no re-pathfind.
	UPathFollowingComponent* PFollowComp = Controller->FindComponentByClass<UPathFollowingComponent>();
	if (PFollowComp == nullptr)
	{
		PFollowComp = NewObject<UPathFollowingComponent>(Controller);
		PFollowComp->RegisterComponentWithWorld(World);
		PFollowComp->Initialize();
	}

	FAIMoveRequest MoveReq(Destination);
	MoveReq.SetUsePathfinding(false);

	FNavPathSharedPtr Path = Result.Path;
	PFollowComp->RequestMove(MoveReq, Path);
}
```

- [ ] **Step 3: Full build to verify it compiles**

Close the editor first (new UCLASS). Run:

```bat
"C:\Program Files\Epic Games\UE_5.3\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "C:\Users\owner\Documents\GitHub\R1\R1.uproject" -waitmutex
```

Expected: `Build succeeded`. If `NavigationRaycast` / `FPathFindingQuery` are unresolved, confirm includes in Step 2 are present and `NavigationSystem` + `AIModule` are in `R1.Build.cs` (they are).

- [ ] **Step 4: Commit**

```bash
git add Source/R1/Library/R1NavSmoothingLibrary.h Source/R1/Library/R1NavSmoothingLibrary.cpp
git commit -m "feat: add UR1NavSmoothingLibrary with NavRaycast string-pulling"
```

---

## Task 2: Wire `R1PlayerController` to the smoothing library (Stage 2 integration)

**Files:**
- Modify: `Source/R1/Player/R1PlayerController.cpp` (include + 4 call sites across 3 methods)

- [ ] **Step 1: Add the include**

In `Source/R1/Player/R1PlayerController.cpp`, near the other project includes (e.g. after the `#include "Item/..."` block, around line 22), add:

```cpp
#include "Library/R1NavSmoothingLibrary.h"
```

- [ ] **Step 2: Replace the move call in `OnSetDestinationTriggered`**

Find (≈ line 282):

```cpp
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CacheDestination);
```

inside the `if (FVector::Dist(CacheDestination, Hit.Location) > 50.0f)` block, and replace with:

```cpp
				UR1NavSmoothingLibrary::SmoothMoveTo(this, CacheDestination);
```

- [ ] **Step 3: Replace the move call in `OnSetDestinationReleased`**

Find (≈ line 314):

```cpp
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CacheDestination);
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CacheDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
```

Replace only the first line:

```cpp
				UR1NavSmoothingLibrary::SmoothMoveTo(this, CacheDestination);
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CacheDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
```

- [ ] **Step 4: Replace both move calls in `ChaseTargetAndAttack`**

There are two `SimpleMoveToLocation` calls in this method (target branch ≈ line 411, interactable branch ≈ line 428). Replace each:

Target branch — find:

```cpp
			CacheDestination = TargetAttackActor->GetActorLocation();
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CacheDestination);
```

replace with:

```cpp
			CacheDestination = TargetAttackActor->GetActorLocation();
			UR1NavSmoothingLibrary::SmoothMoveTo(this, CacheDestination);
```

Interactable branch — find:

```cpp
			CacheDestination = TargetActor->GetActorLocation();
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CacheDestination);
```

replace with:

```cpp
			CacheDestination = TargetActor->GetActorLocation();
			UR1NavSmoothingLibrary::SmoothMoveTo(this, CacheDestination);
```

> Leave the `SimpleMoveToLocation` call in `ResetMovementState()` (line ≈ 509) **unchanged** — that one re-issues a zero-distance move to nail the character in place; smoothing a single-point path there is pointless and we want it to stay stock.

- [ ] **Step 5: Full build to verify it compiles**

```bat
"C:\Program Files\Epic Games\UE_5.3\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "C:\Users\owner\Documents\GitHub\R1\R1.uproject" -waitmutex
```

Expected: `Build succeeded`.

- [ ] **Step 6: Manual PIE check (Stage 2)**

Launch the editor, Play. Click far across an **L-shaped corridor / around a wall corner**.
Expected: the character takes a visibly **straighter, corner-cutting path that stays on the
floor** (never clips through walls) compared to the old stair-step. Click a spot in direct
line of sight → behaves exactly as before. Drag-hold a move → no stutter.

- [ ] **Step 7: Commit**

```bash
git add Source/R1/Player/R1PlayerController.cpp
git commit -m "feat: route player movement through nav string-pulling"
```

---

## Task 3: `R1Player` motion tuning (Stage 3, config-only)

**Files:**
- Modify: `Source/R1/Character/R1Player.h` (add tunable fields)
- Modify: `Source/R1/Character/R1Player.cpp` (apply in ctor)

- [ ] **Step 1: Add tunable UPROPERTY fields to the header**

In `Source/R1/Character/R1Player.h`, inside the `protected:` section (e.g. right after the `Camera` property block, around line 27), add:

```cpp
protected:
	// --- Stage 3: movement feel (tunable in BP) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Feel")
	float SmoothRotationRateYaw = 720.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Feel")
	float SmoothMaxAcceleration = 2048.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Feel")
	float SmoothBrakingDeceleration = 2048.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Feel")
	float SmoothBrakingFriction = 8.f;
```

- [ ] **Step 2: Apply the fields in the ctor**

In `Source/R1/Character/R1Player.cpp`, find (lines 39-40):

```cpp
	GetCharacterMovement()->bOrientRotationToMovement = true;	// 캐릭터의 움직임에 따라 바라보는 방향을 동기화
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
```

Replace with:

```cpp
	// 캐릭터의 움직임에 따라 바라보는 방향을 동기화 (Stage 3: 부드러운 회전/가감속)
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.f, SmoothRotationRateYaw, 0.f);
	MoveComp->MaxAcceleration = SmoothMaxAcceleration;
	MoveComp->BrakingDecelerationWalking = SmoothBrakingDeceleration;
	MoveComp->bUseSeparateBrakingFriction = true;
	MoveComp->BrakingFriction = SmoothBrakingFriction;
```

(`#include "GameFramework/CharacterMovementComponent.h"` is already present at the top of this .cpp — verified at line 8.)

- [ ] **Step 3: Full build to verify it compiles**

Close the editor (header changed → no Live Coding). Run:

```bat
"C:\Program Files\Epic Games\UE_5.3\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "C:\Users\owner\Documents\GitHub\R1\R1.uproject" -waitmutex
```

Expected: `Build succeeded`.

- [ ] **Step 4: Manual PIE check (Stage 3)**

Play. Move around with several quick direction changes.
Expected: the character **turns smoothly toward the new direction** and **starts/stops
cleanly** without a hard skid or an instant snap. If turns feel stiff at sharp corners, raise
`SmoothRotationRateYaw`; if it slides past the click point, raise `SmoothBrakingDeceleration`
/ `SmoothBrakingFriction` — all editable on the BP without recompiling.

- [ ] **Step 5: Commit**

```bash
git add Source/R1/Character/R1Player.h Source/R1/Character/R1Player.cpp
git commit -m "feat: tunable rotation/acceleration for smooth player movement"
```

---

## Task 4: End-to-end verification & Stage 1 note

**Files:** none (verification only)

- [ ] **Step 1: Run the full verification checklist in PIE**

With all three tasks built and the editor running:
1. **Corner cut:** click across an L-corridor → straighter on-navmesh path, no wall clipping.
2. **Line of sight:** click directly visible spot → identical to stock behavior.
3. **Drag move:** hold-and-drag → smooth, no stutter (50-unit gate still throttles).
4. **Chase + attack:** click a monster around a corner → smooth approach, character **stops
   and triggers the attack** at range (confirm `ChaseTargetAndAttack` stop/attack still fires).
5. **Interactable:** click an interactable across a corner → smooth approach + interact.
6. **Reset/teleport:** trigger `ResetMovementState` path (e.g. the death/abort flow) → still
   nails the character in place (unchanged).

- [ ] **Step 2: (Optional, user-applied) Stage 1 navmesh resolution**

Not code. In-editor, on the level's `RecastNavMesh` actor (or Project Settings →
Navigation Mesh), lower **Cell Size** / **Cell Height** (try halving), rebuild navmesh, and
re-run check #1. Finer polygons give the string-pull straighter material to work with.
This is the user's call to apply and tune; the plan does not edit navmesh assets.

- [ ] **Step 3: Final confirmation**

Confirm all checklist items pass. No commit needed (verification only).

---

## Self-Review (author)

- **Spec coverage:** Stage 2 → Tasks 1-2; Stage 3 (config-only) → Task 3; Stage 1 guidance →
  Task 4 Step 2; fallback/error-handling → Task 1 Steps 2 (all null/short-path guards);
  verification → Task 4. AI-monster smoothing correctly absent (out of scope).
- **Placeholders:** none — every code step shows full code; build/commit commands concrete.
- **Type consistency:** `SmoothMoveTo(AController*, const FVector&)` used identically in
  Tasks 1 and 2. Field names `SmoothRotationRateYaw` / `SmoothMaxAcceleration` /
  `SmoothBrakingDeceleration` / `SmoothBrakingFriction` defined in Task 3 Step 1 and used
  verbatim in Step 2.
