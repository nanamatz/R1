# Floor-at-Once Loading Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Load every room of a floor under the loading screen so in-play door transitions become instant teleports with no streaming hitch.

**Architecture:** Decouple room *load* from room *activation*. Streaming all rooms in at floor start runs each room's `DungeonManager::BeginPlay` → `RegisterRoomManager`, which is made **passive** (register + wire doors only). A new **`ActivateRoom(NodeID)`** does combat/lock/teleport, called explicitly on entry. A load counter gates the loading screen on all-rooms-loaded; the thermal/budget streaming machinery is retired (Blueprint-exposed functions kept as deprecated no-op stubs).

**Tech Stack:** Unreal Engine 5.3, C++, single-player. No automated test harness — verification is **(a)** a successful `Build.bat` compile with the editor **closed**, and **(b)** in-editor Play tests at the integration milestone.

**Project build constraints (from CLAUDE.md):**
- Header changes (new `UFUNCTION`/`UPROPERTY`/members) require a full VS2022/Build.bat build with the **editor closed**. Live Coding will not pick them up.
- Build command (run with editor closed):
  ```
  & "<UE_PATH>\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "C:\Users\owner\Documents\GitHub\R1\R1.uproject" -waitmutex
  ```
  Replace `<UE_PATH>` with the UE 5.3 install (the same one prior builds used). "Target is up to date" / exit 0 with no errors = pass.
- Do **not** edit binary `.uasset` / `.umap`.

**Spec:** `docs/superpowers/specs/2026-06-09-floor-at-once-loading-design.md`

---

## File Structure

| File | Responsibility | Action |
|------|----------------|--------|
| `Source/R1/System/R1RoomStreamingSubsystem.h` | Slim API: spawn/unload + deprecated stubs | Modify |
| `Source/R1/System/R1RoomStreamingSubsystem.cpp` | Spawn/unload impl; gut thermal; clear `RoomStates` on unload | Modify |
| `Source/R1/System/R1LoadingSubSystem.h` | Add content-ready gate | Modify |
| `Source/R1/System/R1LoadingSubSystem.cpp` | Hide only when scene done AND content ready | Modify |
| `Source/R1/Map/R1MapGenerator.h` | New: `ActivateRoom`, floor-load driver, counters, marker helper | Modify |
| `Source/R1/Map/R1MapGenerator.cpp` | Split register/activate; whole-floor spawn; rewire call sites | Modify |

> All work is on branch `refactor-asset-preloading-system`. Tasks 1–4 each end in a clean compile. Tasks 5–7 fill `R1MapGenerator.cpp` bodies whose signatures land in Task 4, so the project keeps compiling. Task 8 is the editor Play test.

---

### Task 1: Slim `R1RoomStreamingSubsystem.h`

Retire the thermal state machine but keep all Blueprint-exposed signatures (as deprecated stubs) and the `ER1RoomThermalState` / `FR1RuntimeBudget` types so Blueprint pin types survive.

**Files:**
- Modify: `Source/R1/System/R1RoomStreamingSubsystem.h`

- [ ] **Step 1: Replace `FR1RoomRuntimeState` (lines 62-79) with the slim version**

```cpp
USTRUCT()
struct FR1RoomRuntimeState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UR1RoomDefinitionData> RoomDefinition = nullptr;

	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> StreamingLevel = nullptr;
};
```

Keep `FR1RuntimeBudget` (lines 27-51) and `ER1RoomThermalState` (lines 53-60) exactly as they are — they remain only as Blueprint pin types for the stubs.

- [ ] **Step 2: In the class body, mark the retired functions deprecated and drop the dead private helpers**

Leave `SpawnRoomLevel` and `UnloadAllRooms` as-is. Replace the declarations of `SetRuntimeBudget`, `GetRuntimeBudget`, `QueuePreloadRooms`, `MarkRoomGameplayReady`, `GetRoomState`, `TickRoomCachePolicy`, `MarkRoomAsLeft` with deprecated tags (keep the `UFUNCTION` lines):

```cpp
	UE_DEPRECATED(5.3, "Thermal streaming retired; floor loads whole at once. No-op.")
	UFUNCTION(BlueprintCallable, Category = "Room Streaming", meta = (DeprecatedFunction))
	void SetRuntimeBudget(const FR1RuntimeBudget& InBudget);

	UE_DEPRECATED(5.3, "Thermal streaming retired. Returns default budget.")
	UFUNCTION(BlueprintPure, Category = "Room Streaming", meta = (DeprecatedFunction))
	FR1RuntimeBudget GetRuntimeBudget() const;

	UE_DEPRECATED(5.3, "Whole floor is preloaded; preload-on-demand retired. No-op.")
	UFUNCTION(BlueprintCallable, Category = "Room Streaming", meta = (DeprecatedFunction))
	void QueuePreloadRooms(const TArray<UR1RoomDefinitionData*>& CandidateRooms);

	UE_DEPRECATED(5.3, "Activation moved to AR1MapGenerator::ActivateRoom. No-op.")
	UFUNCTION(BlueprintCallable, Category = "Room Streaming", meta = (DeprecatedFunction))
	void MarkRoomGameplayReady(UR1RoomDefinitionData* RoomDefinition);

	UE_DEPRECATED(5.3, "Thermal state retired. Always returns Hot.")
	UFUNCTION(BlueprintPure, Category = "Room Streaming", meta = (DeprecatedFunction))
	ER1RoomThermalState GetRoomState(UR1RoomDefinitionData* RoomDefinition) const;

	UE_DEPRECATED(5.3, "Cache policy retired. No-op.")
	UFUNCTION(BlueprintCallable, Category = "Room Streaming", meta = (DeprecatedFunction))
	void TickRoomCachePolicy();

	UE_DEPRECATED(5.3, "Rooms stay resident for the whole floor. No-op.")
	UFUNCTION(BlueprintCallable, Category = "Room Streaming", meta = (DeprecatedFunction))
	void MarkRoomAsLeft(UR1RoomDefinitionData* RoomDefinition);
```

Keep `UnloadAllRooms` and the `OnRoomBecameHot` `UPROPERTY(BlueprintAssignable)` declaration (never broadcast, but preserves BP bindings).

- [ ] **Step 3: Remove the dead private members and helpers**

Delete these private declarations (they belonged to the thermal system):
```cpp
	void UnloadRoomInternal(FR1RoomRuntimeState& State);   // keep — see Task 2 (simplified)
	void BeginPreload(UR1RoomDefinitionData* RoomDefinition);   // DELETE
	void TrimPreloadIfNeeded();                                 // DELETE
	UPROPERTY(EditAnywhere, Category = "Room Streaming")
	FR1RuntimeBudget Budget;                                    // KEEP (GetRuntimeBudget returns it)
	UPROPERTY(EditAnywhere, Category = "Room Streaming")
	TObjectPtr<UR1AssetData> GlobalAssetData;                   // DELETE (only BeginPreload used it)
```
Net: delete `BeginPreload`, `TrimPreloadIfNeeded`, and the `GlobalAssetData` member. Keep `UnloadRoomInternal`, `MakeRoomKey`, `Budget`, and `RoomStates`. The `class UR1AssetData;` forward declaration can stay or go (harmless); leave it.

- [ ] **Step 4: Build (editor closed)**

Run the Build.bat command above.
Expected: compiles. (Body changes for the stubs land in Task 2; the linker only needs them after Task 2, so build at the **end of Task 2**, not here, if you prefer a single cycle. If you build now, expect unresolved externals for the stubbed bodies — that is fine to defer.)

> Recommended: do Task 1 + Task 2 back-to-back, then build once.

---

### Task 2: Gut `R1RoomStreamingSubsystem.cpp`

**Files:**
- Modify: `Source/R1/System/R1RoomStreamingSubsystem.cpp`

- [ ] **Step 1: Replace the whole file body with the slim implementation**

```cpp
/**
 * UR1RoomStreamingSubsystem — 슬림 버전.
 * 층(Floor) 단위 전체 로딩 모델로 전환하면서, 룸 레벨 인스턴스의 스폰/언로드만
 * 담당합니다. 기존 열적(thermal) 상태 머신/예산(budget)/선로딩 로직은 폐기되었고,
 * 블루프린트 호환을 위해 시그니처만 deprecated no-op으로 남겨 둡니다.
 */

#include "System/R1RoomStreamingSubsystem.h"
#include "Data/R1RoomDefinitionData.h"
#include "Engine/LevelStreamingDynamic.h"

void UR1RoomStreamingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RoomStates.Reset();
}

void UR1RoomStreamingSubsystem::Deinitialize()
{
	RoomStates.Reset();
	Super::Deinitialize();
}

ULevelStreamingDynamic* UR1RoomStreamingSubsystem::SpawnRoomLevel(UR1RoomDefinitionData* RoomDefinition, FVector Location, FRotator Rotation)
{
	if (!RoomDefinition || RoomDefinition->RoomLevel.IsNull()) return nullptr;

	const FName RoomKey = MakeRoomKey(RoomDefinition);
	FR1RoomRuntimeState& State = RoomStates.FindOrAdd(RoomKey);
	State.RoomDefinition = RoomDefinition;

	if (State.StreamingLevel) return State.StreamingLevel;

	bool bOutSuccess = false;
	State.StreamingLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		this, RoomDefinition->RoomLevel, Location, Rotation, bOutSuccess
	);

	return State.StreamingLevel;
}

void UR1RoomStreamingSubsystem::UnloadAllRooms()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();
	for (ULevelStreaming* Level : StreamingLevels)
	{
		if (ULevelStreamingDynamic* DynamicLevel = Cast<ULevelStreamingDynamic>(Level))
		{
			DynamicLevel->SetShouldBeLoaded(false);
			DynamicLevel->SetShouldBeVisible(false);
			DynamicLevel->SetIsRequestingUnloadAndRemoval(true);
		}
	}

	// [중요] 다음 층에서 SpawnRoomLevel이 '제거 중'인 옛 인스턴스를 재사용(dedup)하지
	// 않도록 룸 상태 캐시를 비웁니다.
	RoomStates.Reset();

	UE_LOG(LogTemp, Warning, TEXT("[RoomStreaming] 이전 층의 모든 방을 메모리에서 해제했습니다."));
}

void UR1RoomStreamingSubsystem::UnloadRoomInternal(FR1RoomRuntimeState& State)
{
	if (State.StreamingLevel)
	{
		State.StreamingLevel->SetIsRequestingUnloadAndRemoval(true);
		State.StreamingLevel = nullptr;
	}
}

FName UR1RoomStreamingSubsystem::MakeRoomKey(const UR1RoomDefinitionData* RoomDefinition) const
{
	if (RoomDefinition == nullptr)
	{
		return NAME_None;
	}
	return FName(*RoomDefinition->GetPrimaryAssetId().ToString());
}

// ---- 이하 deprecated no-op 스텁 (블루프린트 호환 목적; 실제 동작 없음) ----

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void UR1RoomStreamingSubsystem::SetRuntimeBudget(const FR1RuntimeBudget& InBudget) {}

FR1RuntimeBudget UR1RoomStreamingSubsystem::GetRuntimeBudget() const { return Budget; }

void UR1RoomStreamingSubsystem::QueuePreloadRooms(const TArray<UR1RoomDefinitionData*>& CandidateRooms) {}

void UR1RoomStreamingSubsystem::MarkRoomGameplayReady(UR1RoomDefinitionData* RoomDefinition) {}

ER1RoomThermalState UR1RoomStreamingSubsystem::GetRoomState(UR1RoomDefinitionData* RoomDefinition) const
{
	return ER1RoomThermalState::Hot;
}

void UR1RoomStreamingSubsystem::TickRoomCachePolicy() {}

void UR1RoomStreamingSubsystem::MarkRoomAsLeft(UR1RoomDefinitionData* RoomDefinition) {}

PRAGMA_ENABLE_DEPRECATION_WARNINGS
```

> `PRAGMA_DISABLE_DEPRECATION_WARNINGS` around the stub *definitions* prevents the compiler from flagging your own deprecated-function definitions.

- [ ] **Step 2: Build (editor closed)**

Run the Build.bat command.
Expected: compiles and links with no errors. Deprecation warnings only appear at external call sites (Task 7 removes those).

- [ ] **Step 3: Commit**

```bash
git add Source/R1/System/R1RoomStreamingSubsystem.h Source/R1/System/R1RoomStreamingSubsystem.cpp
git commit -m "refactor: slim room streaming subsystem to spawn/unload only

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 3: Content-ready gate in `UR1LoadingSubSystem`

The screen currently hides on the widget animation finishing (`OnSceneFinished`), independent of whether rooms loaded. Gate the hide on **both** the scene finishing **and** content being ready.

**Files:**
- Modify: `Source/R1/System/R1LoadingSubSystem.h`
- Modify: `Source/R1/System/R1LoadingSubSystem.cpp`

- [ ] **Step 1: Add the gate API + flags to the header**

In `R1LoadingSubSystem.h`, add a public function after `OnVisualsCompleted()` (line 39):

```cpp
	// 맵 제너레이터가 "이 층의 모든 방 로드가 끝났다"고 알릴 때 호출. 게이트의 두 번째 조건.
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void NotifyContentReady();
```

Add private state below `LoadingWidget` (after line 23):

```cpp
	// 로딩 화면 해제 게이트: 두 조건이 모두 충족되어야 화면을 내립니다.
	bool bSceneDone = false;     // 위젯 연출(OnSceneFinished) 완료
	bool bContentReady = false;  // 모든 방 로드 완료(NotifyContentReady)

	void TryHideLoadingScreen();
```

- [ ] **Step 2: Reset the gate in `ShowLoadingScreen`**

In `R1LoadingSubSystem.cpp`, at the top of `ShowLoadingScreen` (after `if (!WidgetClass) return;`, line 14):

```cpp
	bSceneDone = false;
	bContentReady = false;
```

- [ ] **Step 3: Replace `OnVisualsCompleted` and add the gate helpers**

Replace the existing `OnVisualsCompleted` (lines 101-105) with:

```cpp
void UR1LoadingSubSystem::OnVisualsCompleted()
{
	bSceneDone = true;
	TryHideLoadingScreen();
}

void UR1LoadingSubSystem::NotifyContentReady()
{
	bContentReady = true;
	TryHideLoadingScreen();
}

void UR1LoadingSubSystem::TryHideLoadingScreen()
{
	if (!bSceneDone || !bContentReady) return;
	if (!LoadingWidget) return; // 이미 내려갔으면 중복 방지

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UR1LoadingSubSystem::HideLoadingScreen, 0.7f, false);
}
```

> The original 0.7s delay is preserved, now fired only when both gate conditions hold.

- [ ] **Step 4: Build (editor closed)**

Run the Build.bat command.
Expected: compiles. (`NotifyContentReady` is called in Task 6; unused-but-defined is fine.)

- [ ] **Step 5: Commit**

```bash
git add Source/R1/System/R1LoadingSubSystem.h Source/R1/System/R1LoadingSubSystem.cpp
git commit -m "feat: gate loading screen hide on content-ready + scene-done

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 4: `R1MapGenerator.h` — declare the new load/activate API

Add the floor-load driver, the activation entry point, the counters, and the marker helper. This is a header change — full build with editor closed.

**Files:**
- Modify: `Source/R1/Map/R1MapGenerator.h`

- [ ] **Step 1: Add a forward declaration**

Near the other forward declarations (after `class UR1AssetData;`, line 10):

```cpp
class AR1PlayerSpawnMarker;
```

- [ ] **Step 2: Declare the public activation + floor-load functions**

Inside `AR1MapGenerator`, in a `public:` section (e.g. right after `GoToNextFloor()` / `IsLastFloor()`, around line 163), add:

```cpp
	// 전체 층의 모든 방을 스폰하고, 전부 로드되면 OnFloorFullyLoaded를 호출합니다.
	void SpawnFloorAndWait();

	// 방 진입 시 명시적으로 호출: 전투 시작/문 잠금/플레이어 텔레포트.
	void ActivateRoom(int32 NodeID);
```

- [ ] **Step 3: Declare the load-callback, completion handler, marker helper, and counters (private)**

In a `private:` section (e.g. near `UpdateMinimapState`, line 166), add:

```cpp
	// 각 룸 레벨 인스턴스의 OnLevelLoaded에 바인딩되는 카운터 콜백.
	UFUNCTION()
	void HandleFloorRoomLoaded();

	// 모든 방 로드가 끝났을 때 1회 실행: 진행도 100%, 시작/복귀 방 활성화, 로딩 게이트 해제.
	void OnFloorFullyLoaded();

	// NodeID 방의 SpawnLocation에 가장 가까운 플레이어 스폰 마커를 찾습니다.
	AR1PlayerSpawnMarker* FindSpawnMarkerForNode(int32 NodeID) const;

	// 층 로딩 진행 카운터
	int32 ExpectedFloorRoomCount = 0;
	int32 LoadedFloorRoomCount = 0;
	bool bFloorActivated = false;

	// 로딩 완료 후 활성화할 방(신규 층=0, 세이브 복귀=저장된 방).
	int32 PendingActivateNodeID = 0;
```

- [ ] **Step 4: Remove the obsolete transition/saved-room callbacks**

Delete these declarations (their streaming-callback role is gone — Task 7 removes the definitions):
- Line 143-144: the `UFUNCTION() void OnSavedRoomLoaded();`
- Line 217-218: the `UFUNCTION() void OnTransitionRoomLoaded();`

Keep `RegisterRoomManager` (line 151) — its definition becomes passive in Task 5.

- [ ] **Step 5: Build — DEFER**

Do not build yet; `R1MapGenerator.cpp` still defines `OnSavedRoomLoaded`/`OnTransitionRoomLoaded` (removed in Task 7) and lacks the new bodies. Build at the end of Task 7.

---

### Task 5: Split `RegisterRoomManager` into passive register + `ActivateRoom`

**Files:**
- Modify: `Source/R1/Map/R1MapGenerator.cpp` (replace `RegisterRoomManager`, lines 623-842)

- [ ] **Step 1: Replace `RegisterRoomManager` (lines 623-842) with the passive version**

```cpp
void AR1MapGenerator::RegisterRoomManager(ADungeonManager* Manager, int32 RoomNodeID)
{
	if (!IsValid(Manager)) return;

	// 1. 위치 또는 명시적 ID로 이 매니저가 몇 번 방인지 식별
	int32 MatchedNodeID = RoomNodeID;
	if (MatchedNodeID == -1)
	{
		for (int32 i = 0; i < GeneratedMap.Num(); ++i)
		{
			if (GeneratedMap[i].SpawnLocation.Equals(Manager->GetActorLocation(), 10.0f))
			{
				MatchedNodeID = i;
				break;
			}
		}
	}

	if (MatchedNodeID == -1 || !GeneratedMap.IsValidIndex(MatchedNodeID)) return;

	ActiveManagers.Add(MatchedNodeID, Manager);

	if (GeneratedMap[MatchedNodeID].RoomDefinition)
	{
		Manager->InitializeRoomData(GeneratedMap[MatchedNodeID].RoomDefinition);
	}

	Manager->RoomNodeID = MatchedNodeID;
	Manager->OnRoomCleared.RemoveDynamic(this, &AR1MapGenerator::OnRoomClearedCallback);
	Manager->OnRoomCleared.AddDynamic(this, &AR1MapGenerator::OnRoomClearedCallback);

	// 2. 모든 문 연결 설정 + 입장 델리게이트 바인딩 (수동/패시브 — 전투/텔레포트 없음)
	for (AR1Door* Door : Manager->RoomDoors)
	{
		if (!IsValid(Door)) continue;

		int32 TargetNode = GetConnectedNodeInDirection(MatchedNodeID, Door->DoorDirection);

		ER1RoomContentType TargetRoomType = ER1RoomContentType::None;
		if (TargetNode != -1 && GeneratedMap.IsValidIndex(TargetNode) && GeneratedMap[TargetNode].RoomDefinition)
		{
			TargetRoomType = GeneratedMap[TargetNode].RoomDefinition->RoomType;
		}

		Door->SetupDoorConnection(TargetNode, TargetRoomType);

		if (TargetNode != -1 && GeneratedMap[TargetNode].RoomDefinition)
		{
			if (GeneratedMap[TargetNode].RoomDefinition->RoomType == ER1RoomContentType::Treasure)
			{
				if (!GeneratedMap[TargetNode].bIsTreasureUnlocked)
				{
					Door->SetKeyLocked(true);
				}
			}
		}

		if (TargetNode != -1)
		{
			Door->OnDoorEntered.RemoveDynamic(this, &AR1MapGenerator::OnPlayerEnteredDoor);
			Door->OnDoorEntered.AddDynamic(this, &AR1MapGenerator::OnPlayerEnteredDoor);
		}
	}

	// 3. 이미 클리어된 방은 문을 열어 둔 시각 상태로. (전투/텔레포트는 ActivateRoom에서)
	if (GeneratedMap[MatchedNodeID].bIsCleared)
	{
		Manager->bIsCleared = true;
		Manager->UnlockRoomDoors();
	}
}
```

- [ ] **Step 2: Add `ActivateRoom` immediately after `RegisterRoomManager`**

```cpp
void AR1MapGenerator::ActivateRoom(int32 NodeID)
{
	if (!GeneratedMap.IsValidIndex(NodeID)) return;

	ADungeonManager* Manager = ActiveManagers.FindRef(NodeID);
	if (!IsValid(Manager))
	{
		UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] ActivateRoom: %d번 방 매니저가 아직 등록되지 않았습니다."), NodeID);
		return;
	}

	const int32 PrevRoomID = CurrentActiveNodeID;

	// 1. 전투 시작 / 문 잠금 (아직 클리어되지 않은 방만)
	if (GeneratedMap[NodeID].bIsCleared)
	{
		Manager->bIsCleared = true;
		Manager->UnlockRoomDoors();
	}
	else
	{
		Manager->LockRoomDoors();
		if (Manager->ClearCondition == ER1RoomClearCondition::Treasure)
		{
			Manager->CompleteRoom();
		}
		else
		{
			Manager->StartRoomCombat();
		}
	}

	// 2. 문을 통한 진입이면, 반대편 문 옆으로 스폰하기 위해 그 문을 찾습니다.
	AR1Door* TargetDoorToSpawnAt = nullptr;
	if (PendingDoorDirection != ER1DoorDirection::None)
	{
		const ER1DoorDirection OppositeDir = GetOppositeDirection(PendingDoorDirection);
		for (AR1Door* Door : Manager->RoomDoors)
		{
			if (IsValid(Door) && Door->DoorDirection == OppositeDir)
			{
				TargetDoorToSpawnAt = Door;
				break;
			}
		}
	}

	// 3. 플레이어 텔레포트
	AR1Player* PlayerCharacter = Cast<AR1Player>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (PlayerCharacter)
	{
		FVector FinalLocation;
		FRotator FinalRotation = FRotator::ZeroRotator;

		if (bIsLoadingFromSave)
		{
			FinalLocation = LoadedPlayerLocation;
			FinalRotation = LoadedPlayerRotation;
			bIsLoadingFromSave = false;
		}
		else if (TargetDoorToSpawnAt)
		{
			const FVector DirectionToCenter =
				(GeneratedMap[NodeID].SpawnLocation - TargetDoorToSpawnAt->GetActorLocation()).GetSafeNormal();
			FinalLocation = TargetDoorToSpawnAt->GetActorLocation() + (DirectionToCenter * 300.0f) + FVector(0.0f, 0.0f, 100.0f);
		}
		else if (AR1PlayerSpawnMarker* Marker = FindSpawnMarkerForNode(NodeID))
		{
			FinalLocation = Marker->GetActorLocation();
			FinalRotation = Marker->GetActorRotation();
		}
		else
		{
			FinalLocation = GeneratedMap[NodeID].SpawnLocation + FVector(0.0f, 0.0f, 150.0f);
		}

		if (AR1PlayerController* PC = Cast<AR1PlayerController>(PlayerCharacter->GetController()))
		{
			PC->ResetMovementState();
		}

		PlayerCharacter->TeleportToRoom(FinalLocation);
		PlayerCharacter->SetActorRotation(FinalRotation);
	}

	// 4. 상태 갱신 + 미니맵 + 오토세이브
	CurrentActiveNodeID = NodeID;
	PendingNodeID = -1;
	PendingDoorDirection = ER1DoorDirection::None;
	InitializedNodeIDs.Add(NodeID);

	UpdateMinimapState(NodeID, PrevRoomID);
	if (OnPlayerMovedRoom.IsBound())
	{
		OnPlayerMovedRoom.Broadcast(NodeID, PrevRoomID);
	}

	TriggerAutoSave();
}
```

> No build here — `FindSpawnMarkerForNode` and the floor-load functions arrive in Task 6, and the call sites in Task 7. Build at the end of Task 7.

---

### Task 6: Floor-load driver + marker helper

**Files:**
- Modify: `Source/R1/Map/R1MapGenerator.cpp` (add functions; `#include` already present for `EngineUtils.h` and `R1PlayerSpawnMarker.h`)

- [ ] **Step 1: Add `SpawnFloorAndWait`, `HandleFloorRoomLoaded`, `OnFloorFullyLoaded`, `FindSpawnMarkerForNode`**

Add after `ActivateRoom`:

```cpp
void AR1MapGenerator::SpawnFloorAndWait()
{
	UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
	if (!RoomSubsystem)
	{
		OnFloorFullyLoaded();
		return;
	}

	bFloorActivated = false;
	ExpectedFloorRoomCount = 0;
	LoadedFloorRoomCount = 0;

	// 1패스: 모든 방 스폰 + 기대 카운트 확정 (실패한 스폰은 카운트에서 제외)
	TArray<ULevelStreamingDynamic*> SpawnedLevels;
	for (const FR1MapNode& Node : GeneratedMap)
	{
		if (!Node.RoomDefinition) continue;

		ULevelStreamingDynamic* Level = RoomSubsystem->SpawnRoomLevel(
			Node.RoomDefinition, Node.SpawnLocation, FRotator::ZeroRotator);
		if (!Level) continue;

		ExpectedFloorRoomCount++;
		SpawnedLevels.Add(Level);
	}

	if (ExpectedFloorRoomCount == 0)
	{
		OnFloorFullyLoaded();
		return;
	}

	// 2패스: 이미 로드된 건 즉시 카운트, 나머지는 OnLevelLoaded에 바인딩
	for (ULevelStreamingDynamic* Level : SpawnedLevels)
	{
		if (Level->IsLevelLoaded())
		{
			LoadedFloorRoomCount++;
		}
		else
		{
			Level->OnLevelLoaded.AddDynamic(this, &AR1MapGenerator::HandleFloorRoomLoaded);
		}
	}

	if (LoadedFloorRoomCount >= ExpectedFloorRoomCount)
	{
		OnFloorFullyLoaded();
	}
}

void AR1MapGenerator::HandleFloorRoomLoaded()
{
	LoadedFloorRoomCount++;
	if (!bFloorActivated && LoadedFloorRoomCount >= ExpectedFloorRoomCount)
	{
		OnFloorFullyLoaded();
	}
}

void AR1MapGenerator::OnFloorFullyLoaded()
{
	if (bFloorActivated) return;
	bFloorActivated = true;

	HighestAchievedProgress = 1.0f;
	OnGenerateProgressUpdated.Broadcast(HighestAchievedProgress);

	if (OnMapGenerated.IsBound())
	{
		OnMapGenerated.Broadcast(GeneratedMap);
	}

	if (UR1LoadingSubSystem* LoadingSubsystem = GetGameInstance()->GetSubsystem<UR1LoadingSubSystem>())
	{
		LoadingSubsystem->NotifyContentReady();
	}

	ActivateRoom(PendingActivateNodeID);
}

AR1PlayerSpawnMarker* AR1MapGenerator::FindSpawnMarkerForNode(int32 NodeID) const
{
	if (!GeneratedMap.IsValidIndex(NodeID)) return nullptr;

	const FVector RoomCenter = GeneratedMap[NodeID].SpawnLocation;
	AR1PlayerSpawnMarker* Best = nullptr;
	double BestDistSq = TNumericLimits<double>::Max();

	for (TActorIterator<AR1PlayerSpawnMarker> It(GetWorld()); It; ++It)
	{
		const double DistSq = FVector::DistSquared(It->GetActorLocation(), RoomCenter);
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = *It;
		}
	}
	return Best;
}
```

> No build yet — Task 7 wires the call sites and removes the old streaming-callback bodies. Build at the end of Task 7.

---

### Task 7: Rewire call sites; remove obsolete callbacks

**Files:**
- Modify: `Source/R1/Map/R1MapGenerator.cpp` (`InitializeMap`, `OnPlayerEnteredDoor`, `LoadMapFromSaveData`, `GoToNextFloor`; delete `OnSavedRoomLoaded` + `OnTransitionRoomLoaded`)

- [ ] **Step 1: `InitializeMap` — route the new-game branch through `SpawnFloorAndWait`**

Replace the `else` block (lines 58-75) with:

```cpp
	else
	{
		HighestAchievedProgress = 0.1f;
		OnGenerateProgressUpdated.Broadcast(HighestAchievedProgress);

		InitializeRoomPools();
		GenerateMap();

		HighestAchievedProgress = 0.5f;
		OnGenerateProgressUpdated.Broadcast(HighestAchievedProgress);

		PendingActivateNodeID = 0;
		SpawnFloorAndWait();
	}
```

- [ ] **Step 2: `OnPlayerEnteredDoor` — activate the (already-resident) room directly**

Replace the entire body (lines 464-519) with:

```cpp
void AR1MapGenerator::OnPlayerEnteredDoor(ER1DoorDirection Direction)
{
	int32 NextNodeID = GetConnectedNodeInDirection(CurrentActiveNodeID, Direction);
	if (NextNodeID == -1) return;
	if (PendingNodeID != -1) return;

	AR1Player* PlayerCharacter = Cast<AR1Player>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (PlayerCharacter)
	{
		if (AR1PlayerController* PC = Cast<AR1PlayerController>(PlayerCharacter->GetController()))
		{
			PC->ResetMovementState();
		}
	}

	if (GeneratedMap[NextNodeID].RoomDefinition &&
		GeneratedMap[NextNodeID].RoomDefinition->RoomType == ER1RoomContentType::Treasure)
	{
		GeneratedMap[NextNodeID].bIsTreasureUnlocked = true;
	}

	PendingNodeID = NextNodeID;
	PendingDoorDirection = Direction;

	// 층 전체가 이미 메모리에 있으므로 스트리밍 대기 없이 즉시 활성화 (= 끊김 없음).
	ActivateRoom(NextNodeID);
}
```

- [ ] **Step 3: `LoadMapFromSaveData` — spawn the whole floor, activate the saved room**

Replace the tail (lines 563-588, from the comment `// 5. 플레이어가 껐을 때...` through the function's closing brace) with:

```cpp
	// 5. 층 전체를 스폰하고, 모두 로드되면 저장된 방을 활성화 (위치는 LoadedPlayerLocation 사용)
	PendingActivateNodeID = CurrentActiveNodeID;
	SpawnFloorAndWait();

	UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 📂 %d층 %d번 방에서 이어서 시작합니다!"), CurrentFloorIndex + 1, CurrentActiveNodeID);
}
```

- [ ] **Step 4: Delete `OnSavedRoomLoaded` (lines 590-597)**

Remove the whole function definition. `LoadMapFromSaveData` no longer references it.

- [ ] **Step 5: `GoToNextFloor` — spawn the whole next floor**

Replace the tail (lines 888-901, from `// 4. 새로운 층의 0번 방 스폰 지시` through the closing brace) with:

```cpp
	// 4. 새 층의 모든 방을 스폰하고, 모두 로드되면 0번(시작) 방을 활성화
	PendingActivateNodeID = 0;
	SpawnFloorAndWait();
}
```

- [ ] **Step 6: Delete `OnTransitionRoomLoaded` (lines 940-958)**

Remove the whole function definition. Nothing binds it any more.

- [ ] **Step 7: Confirm no stale references remain**

Run a search for removed symbols. Expected: zero matches in `R1MapGenerator.cpp`.

```
QueuePreloadRooms | MarkRoomGameplayReady | MarkRoomAsLeft | OnTransitionRoomLoaded | OnSavedRoomLoaded
```

If any remain (e.g. the adjacent-preload blocks that lived inside the old `RegisterRoomManager` — already removed in Task 5), delete them.

- [ ] **Step 8: Full build (editor CLOSED)**

Close the Unreal editor, then run the Build.bat command.
Expected: compiles and links, exit 0. Fix any unresolved symbols or signature mismatches before proceeding.

- [ ] **Step 9: Commit**

```bash
git add Source/R1/Map/R1MapGenerator.h Source/R1/Map/R1MapGenerator.cpp
git commit -m "feat: load whole floor at once; split register/activate

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 8: In-editor integration test

No automated harness — verify by playing. Open the editor (rebuilt DLL loaded) and Play In Editor.

- [ ] **Step 1: First-floor hitch + activation check**

- Start a new run. Confirm the loading screen stays up until the floor is ready, then the player spawns in the start room.
- Walk through every door. **Expect: no hitch / frame-stall on first entry to each room.**
- Confirm monsters spawn **only** in the room you entered (peek adjacent rooms via minimap / fly cam if available — unentered rooms should have no active monsters).

- [ ] **Step 2: Floor transition**

- Clear the boss room, take the portal. Confirm: loading screen appears, previous floor unloads, next floor loads fully, player spawns in the new start room. No leftover monsters from the previous floor, no double-teleport.

- [ ] **Step 3: Save / resume mid-floor**

- Mid-floor (in a non-start room), quit to menu, then Continue. Confirm the player resumes at the saved location in the correct room, and only that room is active.

- [ ] **Step 4: First-combat hitch watch (regression guard)**

The retired label/primary-asset preload was a soft-reference optimization; room hard-refs (monster BP classes referenced by in-level spawners, meshes) chain-load with the sublevel, so this should be fine. **If** you observe a stall on the *first* `StartRoomCombat` of a room, note it — the follow-up is to add a floor-scoped soft-asset preload (union of each room's `PreloadAssetLabels` / `PreloadPrimaryAssets`) folded into the load gate. Do not implement preemptively (YAGNI).

- [ ] **Step 5: Final commit (if any test-driven tweaks were needed)**

```bash
git add -A
git commit -m "fix: address floor-at-once integration test findings

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

## Notes / risks

- **Load-vs-BeginPlay ordering:** `OnFloorFullyLoaded → ActivateRoom` relies on each room's `DungeonManager::BeginPlay` (→ passive `RegisterRoomManager`) having run before its `OnLevelLoaded` fires. This is the same ordering the current `OnTransitionRoomLoaded` path already depends on. `ActivateRoom` logs and bails if the manager is missing, so a violation is visible, not a crash.
- **Memory:** ~7–12 sublevels resident — acceptable per design (single-player Win64).
- **Blueprint stubs:** kept as deprecated no-ops; remove them in a later cleanup once a reference check confirms no Blueprint calls them.
