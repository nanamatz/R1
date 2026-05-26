# Player Spawn Marker Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Allow level designers to place an `AR1PlayerSpawnMarker` Actor in a start room sublevel to control where the player character spawns when beginning a new run or entering a new floor.

**Architecture:** A new lightweight `AR1PlayerSpawnMarker` Actor (editor-visible, no tick, no collision) is placed in the start room sublevel. `AR1MapGenerator::RegisterRoomManager()` queries for this actor via `GetActorOfClass` inside the new-run/new-floor path, using its world transform for the player teleport. If no marker is present the existing centre+Z150 fallback applies unchanged. The save-load and door-transition paths are untouched.

**Tech Stack:** Unreal Engine 5.3, C++17 (MSVC), `UArrowComponent`, `UBillboardComponent`, `UGameplayStatics::GetActorOfClass`.  
**Build command** (from `CLAUDE.md` §2 — substitute your local `<UE_PATH>` and `<ProjectPath>`):
```bat
"<UE_PATH>\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "<ProjectPath>\R1.uproject" -waitmutex
```

---

## File Map

| File | Action | Responsibility |
|---|---|---|
| `Source/R1/Map/R1PlayerSpawnMarker.h` | **Create** | Declares `AR1PlayerSpawnMarker` with arrow + billboard components |
| `Source/R1/Map/R1PlayerSpawnMarker.cpp` | **Create** | Constructs components; sets no-tick, no-collision |
| `Source/R1/Map/R1MapGenerator.cpp` | **Modify** | Adds include + replaces hardcoded spawn fallback with marker lookup |
| `Content/Blueprints/Map/BP_PlayerSpawnMarker.uasset` | **Create (editor)** | Blueprint child for drag-and-drop placement in sublevels |

No changes to `Build.cs`, `R1MapGenerator.h`, or `UR1RoomDefinitionData`.

---

### Task 1: Create `AR1PlayerSpawnMarker` header

**Files:**
- Create: `Source/R1/Map/R1PlayerSpawnMarker.h`

- [ ] **Step 1: Write the header file**

```cpp
// Source/R1/Map/R1PlayerSpawnMarker.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "R1PlayerSpawnMarker.generated.h"

/**
 * Lightweight editor marker. Place one in a start room sublevel to designate
 * where the player character spawns when beginning a new run or a new floor.
 * AR1MapGenerator reads this actor's world transform in RegisterRoomManager().
 * No runtime logic — no tick, no collision.
 */
UCLASS(Blueprintable)
class R1_API AR1PlayerSpawnMarker : public AActor
{
    GENERATED_BODY()

public:
    AR1PlayerSpawnMarker();

protected:
    /** Shows player facing direction in the editor viewport. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Marker")
    TObjectPtr<class UArrowComponent> ArrowComponent;

    /** Editor-only sprite for easy selection in the viewport. Hidden in-game. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Marker")
    TObjectPtr<class UBillboardComponent> BillboardComponent;
};
```

- [ ] **Step 2: Verify placement**

Confirm `R1PlayerSpawnMarker.h` sits alongside the existing `R1MapGenerator.h` and `R1Portal.h` inside `Source/R1/Map/`.

---

### Task 2: Create `AR1PlayerSpawnMarker` implementation and build

**Files:**
- Create: `Source/R1/Map/R1PlayerSpawnMarker.cpp`

- [ ] **Step 1: Write the implementation file**

```cpp
// Source/R1/Map/R1PlayerSpawnMarker.cpp
#include "Map/R1PlayerSpawnMarker.h"
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"

AR1PlayerSpawnMarker::AR1PlayerSpawnMarker()
{
    PrimaryActorTick.bCanEverTick = false;

    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
    ArrowComponent->ArrowSize = 2.0f;
    ArrowComponent->ArrowColor = FColor::Green;
    SetRootComponent(ArrowComponent);

    BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillboardComponent"));
    BillboardComponent->SetupAttachment(ArrowComponent);
    BillboardComponent->bIsScreenSizeScaled = true;
}
```

- [ ] **Step 2: Close the UE editor (header change requires full rebuild)**

Closing is required because `UCLASS` macro changes are not supported by Live Coding.

- [ ] **Step 3: Build**

```bat
"<UE_PATH>\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "<ProjectPath>\R1.uproject" -waitmutex
```

Expected: build completes with no errors. UBT will discover both new files automatically — no `Build.cs` edit needed.

- [ ] **Step 4: Commit**

```
git add Source/R1/Map/R1PlayerSpawnMarker.h Source/R1/Map/R1PlayerSpawnMarker.cpp
git commit -m "feat: add AR1PlayerSpawnMarker editor marker actor"
```

---

### Task 3: Wire the marker into `AR1MapGenerator::RegisterRoomManager()`

**Files:**
- Modify: `Source/R1/Map/R1MapGenerator.cpp`

- [ ] **Step 1: Add the include**

Open `Source/R1/Map/R1MapGenerator.cpp`. After the last existing `#include` line (currently `#include "System/R1LoadingSubSystem.h"` and `#include "UI/System/R1LoadingScreenWidget.h"` near line 20), add:

```cpp
#include "Map/R1PlayerSpawnMarker.h"
```

- [ ] **Step 2: Replace the hardcoded spawn fallback**

Find the `else` branch inside the `if (PendingNodeID == -1)` → `if (PlayerCharacter)` block. It currently reads:

```cpp
else
{
    FinalLocation = GeneratedMap[MatchedNodeID].SpawnLocation + FVector(0.0f, 0.0f, 150.0f);
}
```

Replace it with:

```cpp
else
{
    // Look for a designer-placed spawn marker in the freshly loaded sublevel.
    AActor* SpawnMarker = UGameplayStatics::GetActorOfClass(GetWorld(), AR1PlayerSpawnMarker::StaticClass());
    if (SpawnMarker)
    {
        FinalLocation = SpawnMarker->GetActorLocation();
        FinalRotation = SpawnMarker->GetActorRotation();
    }
    else
    {
        // Fallback: room streaming origin + Z offset (preserves original behaviour).
        FinalLocation = GeneratedMap[MatchedNodeID].SpawnLocation + FVector(0.0f, 0.0f, 150.0f);
    }
}
```

For orientation, the full surrounding context looks like this (approximately lines 720–745):

```cpp
if (PendingNodeID == -1)
{
    if (PlayerCharacter)
    {
        FVector FinalLocation;
        FRotator FinalRotation = FRotator::ZeroRotator;

        if (bIsLoadingFromSave)
        {
            FinalLocation = LoadedPlayerLocation;
            FinalRotation = LoadedPlayerRotation;
            bIsLoadingFromSave = false;
            UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 세이브된 위치로 플레이어를 복구합니다: %s"), *FinalLocation.ToString());
        }
        else   // <-- replace this block
        {
            // ... new marker-lookup code
        }

        PlayerCharacter->TeleportToRoom(FinalLocation);
        PlayerCharacter->SetActorRotation(FinalRotation);
        // ... rest unchanged
    }
}
```

`UGameplayStatics` is already included in `R1MapGenerator.cpp` — no additional include needed.

- [ ] **Step 3: Build (`.cpp`-only change — Live Coding is fine if the editor is open)**

Option A — with editor open: Ctrl+Alt+F11 (Live Coding).  
Option B — editor closed:
```bat
"<UE_PATH>\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "<ProjectPath>\R1.uproject" -waitmutex
```

Expected: build succeeds with no errors or new warnings.

- [ ] **Step 4: Commit**

```
git add Source/R1/Map/R1MapGenerator.cpp
git commit -m "feat: spawn player at AR1PlayerSpawnMarker on new run/floor"
```

---

### Task 4: Create Blueprint child and verify all paths in-editor

**Files:**
- Create (in editor): `Content/Blueprints/Map/BP_PlayerSpawnMarker.uasset`

- [ ] **Step 1: Create `BP_PlayerSpawnMarker`**

1. Launch the project in the UE editor.
2. In the Content Browser, navigate to `Content/Blueprints/` (create a `Map` subfolder if it doesn't exist).
3. Right-click → **Blueprint Class** → search for `R1PlayerSpawnMarker` → select it → name it `BP_PlayerSpawnMarker`.
4. Open it; confirm the green `ArrowComponent` and `BillboardComponent` are visible in the viewport.
5. Save and close the Blueprint editor.

- [ ] **Step 2: Test — marker present, new run**

1. Open the start room sublevel (a sublevel whose `UR1RoomDefinitionData.RoomType == Start`).
2. Drag `BP_PlayerSpawnMarker` into the level; position it at the desired spawn location and rotate the arrow to face inward.
3. Save the sublevel.
4. **Delete any existing save file** so the game starts a fresh run (saves are in `Saved/SaveGames/`).
5. Play-in-Editor (PIE).

Expected: Player spawns at the marker's world position facing the marker's rotation.

- [ ] **Step 3: Test — no marker, new run (fallback)**

1. Delete `BP_PlayerSpawnMarker` from the sublevel and save.
2. Delete any existing save file. PIE.

Expected: Player spawns at the room's streaming origin + `(0, 0, 150)` — identical to pre-feature behaviour.

3. Re-place the marker when done.

- [ ] **Step 4: Test — save-load path unaffected**

1. With the marker placed, start a new run, walk a few steps (change position), then save and quit.
2. Relaunch and continue the run.

Expected: Player spawns at the saved position, **not** at the marker position.

- [ ] **Step 5: Test — door-transition path unaffected**

1. Run the game. Move through a door to an adjacent room.

Expected: Player appears near the door entry as before; marker is not consulted.

- [ ] **Step 6: Commit the Blueprint asset**

```
git add Content/Blueprints/Map/BP_PlayerSpawnMarker.uasset
git commit -m "feat: add BP_PlayerSpawnMarker for start room player spawn point"
```
