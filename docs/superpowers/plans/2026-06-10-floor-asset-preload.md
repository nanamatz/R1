# Floor Asset Preload Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Async-load each floor's designer-listed heavy assets during the loading screen (gated with a timeout fallback) so first-combat no longer hitches.

**Architecture:** All changes live in `AR1MapGenerator`. A new helper gathers the de-duplicated preload set from every room PDA's existing `PreloadAssetLabels` / `PreloadPrimaryAssets` fields and issues a single `RequestAsyncLoad`, kept alive by a member `TSharedPtr<FStreamableHandle>`. The existing `WaitForNavMeshThenActivate` poll loop gains one more readiness condition (preload complete) before it releases the loading gate. The handle is reset at the start of each new floor's preload, releasing the prior floor's assets.

**Tech Stack:** Unreal Engine 5.3, C++ (MSVC/UBT), `UAssetManager` / `FStreamableManager`, GAS-adjacent gameplay code. No automated test harness exists for this flow — verification is **compile (UBT) + in-editor observation**, per the design spec.

---

## Testing Note (read first)

This is Unreal gameplay/loading code with **no unit-test runner**. The "test" gate for each code task is a successful **UBT Development editor build** (a header member is added, so Live Coding is NOT sufficient — a full VS2022/UBT build is required). Behavioral verification is a single manual in-editor pass in the final task. Replace `<UE_PATH>` with your UE 5.3 install (commonly `C:\Program Files\Epic Games\UE_5.3`).

Build command (used as the per-task pass/fail check):

```bat
"<UE_PATH>\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "C:\Users\owner\Documents\GitHub\R1\R1.uproject" -waitmutex
```
Expected: `Build succeeded` with exit code 0.

Close the editor before building (header change).

---

## File Structure

- `Source/R1/Map/R1MapGenerator.h` — add a forward declaration, one member, one private helper declaration.
- `Source/R1/Map/R1MapGenerator.cpp` — add includes, implement the helper, call it in `SpawnFloorAndWait`, extend the gate in `WaitForNavMeshThenActivate`.

No other files change. The room PDA fields (`PreloadAssetLabels`, `PreloadPrimaryAssets`) already exist on `UR1RoomDefinitionData` and are not modified.

---

## Task 1: Header — forward decl, member, helper declaration

**Files:**
- Modify: `Source/R1/Map/R1MapGenerator.h`

This task only adds declarations. Nothing calls the helper yet, so the project still compiles and links.

- [ ] **Step 1: Add the forward declaration**

In `Source/R1/Map/R1MapGenerator.h`, find the existing forward declarations near the top (currently around lines 9-11):

```cpp
class UR1RoomDefinitionData;
class UR1AssetData;
class AR1PlayerSpawnMarker;
```

Add `FStreamableHandle` below them:

```cpp
class UR1RoomDefinitionData;
class UR1AssetData;
class AR1PlayerSpawnMarker;
struct FStreamableHandle;
```

- [ ] **Step 2: Add the member and helper declaration**

Find the private section that declares the navmesh-wait members (currently around lines 202-207):

```cpp
	// 네비메시 비동기 빌드 완료를 기다리는 폴링 타이머/카운터.
	FTimerHandle NavBuildWaitTimer;
	int32 NavBuildWaitTicks = 0;

	// 로딩 완료 후 활성화할 방(신규 층=0, 세이브 복귀=저장된 방).
	int32 PendingActivateNodeID = 0;
```

Insert directly after that block:

```cpp
	// 이 층에서 미리 로드한 에셋들의 스트리밍 핸들. 핸들이 살아있는 동안 에셋이 메모리에 상주한다.
	// (UObject가 아니므로 UPROPERTY로 두지 않는다.)
	TSharedPtr<FStreamableHandle> FloorPreloadHandle;

	// 층의 모든 방 PDA에서 프리로드 대상(라벨/Primary Asset)을 모아 단일 비동기 로드를 시작한다.
	// 이전 핸들은 이 함수 진입 시 해제되어 이전 층 에셋이 GC 대상이 된다.
	void StartFloorAssetPreload();
```

- [ ] **Step 3: Build to verify it still compiles**

Run:
```bat
"<UE_PATH>\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "C:\Users\owner\Documents\GitHub\R1\R1.uproject" -waitmutex
```
Expected: `Build succeeded`, exit code 0. (A declared-but-undefined member function is legal as long as it is never referenced — it is not referenced yet.)

- [ ] **Step 4: Commit**

```bash
git add Source/R1/Map/R1MapGenerator.h
git commit -m "feat : declare floor asset preload handle and helper"
```

---

## Task 2: Implement the preload helper and kick it off

**Files:**
- Modify: `Source/R1/Map/R1MapGenerator.cpp`

- [ ] **Step 1: Add the required includes**

In `Source/R1/Map/R1MapGenerator.cpp`, find the existing include block. After the existing line:

```cpp
#include "System/R1RoomStreamingSubsystem.h"
```

add these three includes (the asset-manager and streamable-manager headers, plus the project asset manager that exposes the loaded global asset data):

```cpp
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "System/R1AssetManager.h"
```

(`Data/R1AssetData.h` and `Data/R1RoomDefinitionData.h` are already included in this file.)

- [ ] **Step 2: Implement `StartFloorAssetPreload`**

Add this function definition. Place it immediately **above** `void AR1MapGenerator::SpawnFloorAndWait()` (currently around line 742):

```cpp
void AR1MapGenerator::StartFloorAssetPreload()
{
	// 이전 층 프리로드 핸들을 해제해 이전 층 에셋을 GC 대상으로 만든다(메모리를 한 층 분량으로 제한).
	FloorPreloadHandle.Reset();

	UAssetManager& AssetManager = UAssetManager::Get();
	UR1AssetData* AssetData = UR1AssetManager::GetLoadedAssetData();

	// 중복 경로 제거용. 여러 방이 같은 라벨/에셋을 가리켜도 한 번만 로드한다.
	TSet<FSoftObjectPath> UniquePaths;

	for (const FR1MapNode& Node : GeneratedMap)
	{
		if (!Node.RoomDefinition) continue;

		// 1) 라벨 → 전역 AssetData에서 소프트 경로로 해석
		if (AssetData)
		{
			for (const FName& Label : Node.RoomDefinition->PreloadAssetLabels)
			{
				if (Label.IsNone()) continue;

				const FAssetSet& Set = AssetData->GetAssetSetByLabel(Label);
				for (const FAssetEntry& Entry : Set.AssetEntries)
				{
					if (Entry.AssetPath.IsValid())
					{
						UniquePaths.Add(Entry.AssetPath);
					}
				}
			}
		}

		// 2) Primary Asset Id → AssetManager에서 소프트 경로로 해석
		for (const FPrimaryAssetId& AssetId : Node.RoomDefinition->PreloadPrimaryAssets)
		{
			if (!AssetId.IsValid()) continue;

			const FSoftObjectPath Path = AssetManager.GetPrimaryAssetPath(AssetId);
			if (Path.IsValid())
			{
				UniquePaths.Add(Path);
			}
			else
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[MapGenerator] 프리로드 Primary Asset 경로 해석 실패: %s"), *AssetId.ToString());
			}
		}
	}

	if (UniquePaths.Num() == 0)
	{
		// 로드할 게 없으면 핸들은 null로 둔다(게이트는 null을 '완료'로 취급).
		return;
	}

	const TArray<FSoftObjectPath> PathsToLoad = UniquePaths.Array();
	FloorPreloadHandle = AssetManager.GetStreamableManager().RequestAsyncLoad(PathsToLoad);

	UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 층 에셋 프리로드 시작: %d개 경로"), PathsToLoad.Num());
}
```

- [ ] **Step 3: Call it at the top of `SpawnFloorAndWait`**

Find the start of `SpawnFloorAndWait` (currently around line 742):

```cpp
void AR1MapGenerator::SpawnFloorAndWait()
{
	UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
```

Insert the preload kickoff as the first statement of the body, so disk I/O overlaps the slow level streaming that follows:

```cpp
void AR1MapGenerator::SpawnFloorAndWait()
{
	// 층 에셋 비동기 프리로드 시작. 아래 레벨 스트리밍(AddToWorld)과 병렬로 진행되므로
	// 정상 경우 추가 대기 시간이 거의 없다. 완료 게이트는 WaitForNavMeshThenActivate에서 처리.
	StartFloorAssetPreload();

	UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
```

- [ ] **Step 4: Build to verify it compiles and links**

Run:
```bat
"<UE_PATH>\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "C:\Users\owner\Documents\GitHub\R1\R1.uproject" -waitmutex
```
Expected: `Build succeeded`, exit code 0.

- [ ] **Step 5: Commit**

```bash
git add Source/R1/Map/R1MapGenerator.cpp
git commit -m "feat : gather and async-load floor preload assets at floor spawn"
```

---

## Task 3: Fold preload-completion into the loading gate

**Files:**
- Modify: `Source/R1/Map/R1MapGenerator.cpp`

At this point the assets load, but the loading screen does not yet wait for them. This task adds the wait to the existing navmesh poll loop in `WaitForNavMeshThenActivate`.

- [ ] **Step 1: Add the preload condition to the re-poll check**

In `WaitForNavMeshThenActivate` (currently around lines 872-895), find the navmesh poll block:

```cpp
		// 방에 navmesh가 아직 없거나, 빌드가 진행 중(타일 일부만 완성)이면 계속 대기.
		const bool bStillBuilding = UNavigationSystemV1::IsNavigationBeingBuilt(World);
		if (!bRoomNavigable || bStillBuilding)
		{
			World->GetTimerManager().SetTimer(
				NavBuildWaitTimer, this, &AR1MapGenerator::WaitForNavMeshThenActivate, NavBuildPollInterval, false);
			return;
		}
```

Replace it with a version that also waits on the preload handle (null handle or `HasLoadCompleted()` counts as ready):

```cpp
		// 방에 navmesh가 아직 없거나, 빌드가 진행 중(타일 일부만 완성)이면 계속 대기.
		const bool bStillBuilding = UNavigationSystemV1::IsNavigationBeingBuilt(World);

		// 에셋 프리로드 완료 여부도 같은 게이트에서 함께 기다린다.
		// 핸들이 없으면(로드할 게 없었으면) 완료로 취급한다.
		const bool bPreloadReady = (!FloorPreloadHandle.IsValid()) || FloorPreloadHandle->HasLoadCompleted();

		if (!bRoomNavigable || bStillBuilding || !bPreloadReady)
		{
			World->GetTimerManager().SetTimer(
				NavBuildWaitTimer, this, &AR1MapGenerator::WaitForNavMeshThenActivate, NavBuildPollInterval, false);
			return;
		}
```

- [ ] **Step 2: Log when proceeding with an incomplete preload on timeout**

Find the timeout block (currently around lines 897-903):

```cpp
	if (bTimedOut)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[MapGenerator] navmesh 대기 타임아웃(%.1fs) — %d번 방에 navmesh가 생성되지 않았습니다. ")
			TEXT("해당 방 레벨의 NavMeshBoundsVolume가 바닥을 감싸는지 확인하세요."),
			NavBuildMaxTicks * NavBuildPollInterval, PendingActivateNodeID);
	}
```

Add a preload-specific warning right after the closing brace of that `if (bTimedOut)` block:

```cpp
	if (bTimedOut && FloorPreloadHandle.IsValid() && !FloorPreloadHandle->HasLoadCompleted())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[MapGenerator] 에셋 프리로드가 끝나기 전에 타임아웃 — 프리로드를 기다리지 않고 진행합니다."));
	}
```

(Leaving the handle alive: even though we proceed, the in-flight load keeps streaming and the assets still become resident shortly after; we simply stopped *blocking* on it.)

- [ ] **Step 3: Build to verify it compiles**

Run:
```bat
"<UE_PATH>\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "C:\Users\owner\Documents\GitHub\R1\R1.uproject" -waitmutex
```
Expected: `Build succeeded`, exit code 0.

- [ ] **Step 4: Commit**

```bash
git add Source/R1/Map/R1MapGenerator.cpp
git commit -m "feat : hold loading gate until floor preload completes (timeout fallback)"
```

---

## Task 4: In-editor verification

**Files:** none (manual verification).

No code changes. Confirm behavior in the editor. Requires designer data in at least one room PDA.

- [ ] **Step 1: Populate one room's preload fields**

In the editor, open one Combat room PDA (a `UR1RoomDefinitionData` asset). Add that room's monster/VFX/SFX assets to `PreloadAssetLabels` (existing AssetData labels) and/or `PreloadPrimaryAssets`. Save the asset.

- [ ] **Step 2: Verify the preload is issued at floor spawn**

Play in editor and enter the floor. In the Output Log, confirm a line:
`[MapGenerator] 층 에셋 프리로드 시작: N개 경로`
appears during loading (issued before the loading screen clears).

- [ ] **Step 3: Verify the gate waits for it**

Confirm the loading screen does not clear until after the preload completes — i.e. you should NOT see the timeout warning `[MapGenerator] 에셋 프리로드가 끝나기 전에 타임아웃` in the normal case, and gameplay input/teleport happens only after content-ready. Confirm first combat in that room no longer hitches on first monster spawn (compare against a room with empty fields).

- [ ] **Step 4: Verify the empty-field no-op path**

Enter the floor via a room whose fields are empty (or a floor with no populated rooms). Confirm loading completes normally with no preload start line and no hang.

- [ ] **Step 5: Verify the invalid-id path does not hang**

Temporarily set a `PreloadPrimaryAssets` entry to an invalid id. Confirm `프리로드 Primary Asset 경로 해석 실패` is logged and loading still completes. Revert the bad entry afterward.

- [ ] **Step 6: Verify cross-floor memory release**

Advance two floors (`GoToNextFloor`). Confirm a fresh `프리로드 시작` line each floor (the previous handle is reset at the top of `StartFloorAssetPreload`), and memory does not grow unbounded across floors.

---

## Self-Review Notes

- **Spec coverage:** data source (designer fields) → Task 2 Step 2; kickoff overlapping streaming → Task 2 Step 3; gate via existing poll loop → Task 3 Step 1; timeout fallback → Task 3 Step 2; lifetime/release on transition → Task 2 Step 2 (`FloorPreloadHandle.Reset()` at helper entry, which runs for every floor including `GoToNextFloor`); untouched `R1LoadingSubSystem` and PDA struct → confirmed (file structure). All spec requirements map to a task.
- **Deviation from spec (intentional, equivalent):** the handle reset is done at the top of `StartFloorAssetPreload` rather than inside `GoToNextFloor`. Same effect ("release previous floor's preload before starting the next"), DRY, and covers every entry path (fresh generate, save-restore, next-floor) instead of just `GoToNextFloor`.
- **Type consistency:** `FloorPreloadHandle` is `TSharedPtr<FStreamableHandle>` everywhere; readiness checked via `.IsValid()` + `->HasLoadCompleted()`; helper named `StartFloorAssetPreload()` in both the header decl (Task 1) and definition/call (Task 2). `GetAssetSetByLabel` returns `const FAssetSet&` with `AssetEntries[].AssetPath` (verified against `R1AssetData.h`). `GetLoadedAssetData()` is the static accessor on `UR1AssetManager`.
- **Build requirement:** Task 1 adds a header member → full UBT build, not Live Coding (called out in the design spec).
