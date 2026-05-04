# Player Location Persistence Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Save and restore the player's exact world location and rotation when loading a game.

**Architecture:** Update the `UR1PlayerSaveGame` data structure to include spatial data, modify `UR1SaveSystem` to capture/pass this data, and update `AR1MapGenerator` to prioritize these saved coordinates over default room-center spawning.

**Tech Stack:** C++, Unreal Engine 5 (SaveGame, Level Streaming)

---

### Task 1: Update Save Data Structure

**Files:**
- Modify: `Source/R1/System/R1PlayerSaveGame.h`

- [ ] **Step 1: Add PlayerLocation and PlayerRotation fields to UR1PlayerSaveGame**

```cpp
// Source/R1/System/R1PlayerSaveGame.h

UCLASS()
class R1_API UR1PlayerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// ... existing stats ...

	// [Added] 플레이어 위치 및 회전 데이터
	UPROPERTY(BlueprintReadWrite)
	FVector PlayerLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FRotator PlayerRotation = FRotator::ZeroRotator;

    // ... existing map data ...
};
```

- [ ] **Step 2: Verify compilation**

Run: `dotnet build` (or equivalent Unreal build command)
Expected: SUCCESS

- [ ] **Step 3: Commit**

```bash
git add Source/R1/System/R1PlayerSaveGame.h
git commit -m "feat: add PlayerLocation and PlayerRotation to save game data"
```

---

### Task 2: Capture Spatial Data during Save

**Files:**
- Modify: `Source/R1/System/R1SaveSystem.cpp`

- [ ] **Step 1: Update SaveCurrentRun to capture player transform**

```cpp
// Source/R1/System/R1SaveSystem.cpp (inside SaveCurrentRun)

void UR1SaveSystem::SaveCurrentRun(AR1Player* Player, AR1MapGenerator* MapGenerator)
{
    // ... setup SaveObj ...

	// 1. 플레이어 상태 저장
	if (Player)
	{
        // [Added] 위치 및 회전 저장
        SaveObj->PlayerLocation = Player->GetActorLocation();
        SaveObj->PlayerRotation = Player->GetActorRotation();

		if (AR1PlayerState* PS = Cast<AR1PlayerState>(Player->GetPlayerState()))
        // ... rest of logic ...
    }
    // ...
}
```

- [ ] **Step 2: Verify compilation**

Run: `dotnet build`
Expected: SUCCESS

- [ ] **Step 3: Commit**

```bash
git add Source/R1/System/R1SaveSystem.cpp
git commit -m "feat: capture player location and rotation during save"
```

---

### Task 3: Update MapGenerator to Handle Saved Location

**Files:**
- Modify: `Source/R1/Map/R1MapGenerator.h`
- Modify: `Source/R1/Map/R1MapGenerator.cpp`

- [ ] **Step 1: Add tracking variables to R1MapGenerator.h**

```cpp
// Source/R1/Map/R1MapGenerator.h

class R1_API AR1MapGenerator : public AActor
{
    // ...
public:
	// 세이브 데이터로부터 맵을 복원하는 함수
	UFUNCTION(BlueprintCallable, Category = "Map Generation")
	void LoadMapFromSaveData(const TArray<struct FR1MapNodeSaveData>& SavedNodes, int32 SavedFloorIndex, int32 SavedActiveNodeID, FVector SavedLocation = FVector::ZeroVector, FRotator SavedRotation = FRotator::ZeroRotator);

    // ...
private:
    // [Added] 세이브에서 불러온 위치 정보 보관용
    bool bIsLoadingFromSave = false;
    FVector LoadedPlayerLocation = FVector::ZeroVector;
    FRotator LoadedPlayerRotation = FRotator::ZeroRotator;
};
```

- [ ] **Step 2: Update LoadMapFromSaveData implementation in R1MapGenerator.cpp**

```cpp
// Source/R1/Map/R1MapGenerator.cpp

void AR1MapGenerator::LoadMapFromSaveData(const TArray<FR1MapNodeSaveData>& SavedNodes, int32 SavedFloorIndex, int32 SavedActiveNodeID, FVector SavedLocation, FRotator SavedRotation)
{
	CurrentFloorIndex = SavedFloorIndex;
	CurrentActiveNodeID = SavedActiveNodeID;
	PendingNodeID = -1; 

    // [Added] 위치 정보 저장
    bIsLoadingFromSave = true;
    LoadedPlayerLocation = SavedLocation;
    LoadedPlayerRotation = SavedRotation;

    // ... existing restoration logic ...
}
```

- [ ] **Step 3: Update RegisterRoomManager to use saved location**

```cpp
// Source/R1/Map/R1MapGenerator.cpp (inside RegisterRoomManager)

	if (PendingNodeID == -1)
	{
		if (PlayerCharacter)
		{
            FVector FinalLocation;
            FRotator FinalRotation = FRotator::ZeroRotator;

            // [Modified] 세이브 로딩 중이라면 저장된 위치 사용
            if (bIsLoadingFromSave)
            {
                FinalLocation = LoadedPlayerLocation;
                FinalRotation = LoadedPlayerRotation;
                bIsLoadingFromSave = false; // 일회성 처리
                UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 세이브된 위치로 플레이어를 복구합니다: %s"), *FinalLocation.ToString());
            }
            else
            {
                FinalLocation = GeneratedMap[MatchedNodeID].SpawnLocation + FVector(0.0f, 0.0f, 150.0f);
            }

			PlayerCharacter->TeleportToRoom(FinalLocation);
            PlayerCharacter->SetActorRotation(FinalRotation);

            // ... rest of logic ...
        }
    }
```

- [ ] **Step 4: Verify compilation**

Run: `dotnet build`
Expected: SUCCESS

- [ ] **Step 5: Commit**

```bash
git add Source/R1/Map/R1MapGenerator.h Source/R1/Map/R1MapGenerator.cpp
git commit -m "feat: prioritize saved player location in MapGenerator"
```

---

### Task 4: Connect SaveSystem Load to MapGenerator

**Files:**
- Modify: `Source/R1/System/R1SaveSystem.cpp`

- [ ] **Step 1: Pass saved spatial data in LoadCurrentRun**

```cpp
// Source/R1/System/R1SaveSystem.cpp (inside LoadCurrentRun)

bool UR1SaveSystem::LoadCurrentRun(AR1Player* Player, AR1MapGenerator* MapGenerator)
{
	if (!HasSavedRun()) return false;

	UR1PlayerSaveGame* SaveObj = Cast<UR1PlayerSaveGame>(UGameplayStatics::LoadGameFromSlot(RunSaveSlotName, RunSaveUserIndex));
	if (!SaveObj) return false;

    // ... player stats injection ...

	if (MapGenerator)
	{
        // [Modified] 위치와 회전값도 함께 넘겨줌
		MapGenerator->LoadMapFromSaveData(SaveObj->SavedMapNodes, SaveObj->CurrentFloorIndex, SaveObj->CurrentActiveNodeID, SaveObj->PlayerLocation, SaveObj->PlayerRotation);
	}

    // ... inventory restoration ...
    return true;
}
```

- [ ] **Step 2: Final Verification & Commit**

Run: `dotnet build`
Expected: SUCCESS

```bash
git add Source/R1/System/R1SaveSystem.cpp
git commit -m "feat: pass saved spatial data from SaveSystem to MapGenerator"
```
