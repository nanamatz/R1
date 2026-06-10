#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "R1Define.h"
#include "R1MapGenerator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMapGenerateProgress, float, CurrentProgress);
class UR1RoomDefinitionData;
class UR1AssetData;
class AR1PlayerSpawnMarker;

USTRUCT(BlueprintType)
struct FFloorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Settings")
	FName StartRoomLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Settings")
	FName CombatRoomLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Settings")
	FName BossRoomLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Settings")
	FName TreasureRoomLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Settings")
	FName ShopRoomLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Settings")
	FName RefreshRoomLabel;
};

UENUM(BlueprintType)
enum class ER1DoorDirection : uint8
{
	North	UMETA(DisplayName = "North (Y+1)"),
	South	UMETA(DisplayName = "South (Y-1)"),
	East	UMETA(DisplayName = "East (X+1)"),
	West	UMETA(DisplayName = "West (X-1)"),
	None	UMETA(DisplayName = "None")
};


USTRUCT(BlueprintType)
struct FR1MapNode
{
	GENERATED_BODY()
public:

	// 이 방의 고유 번호 (예: 0, 1, 2...)
	UPROPERTY(BlueprintReadOnly)
	int32 NodeID = -1;

	// 이 방에 할당된 룸 데이터 (PDA)
	UPROPERTY(BlueprintReadOnly)
	UR1RoomDefinitionData* RoomDefinition = nullptr;

	// 논리적으로 연결된 다음 방의 번호들
	UPROPERTY(BlueprintReadOnly)
	TArray<int32> ConnectedNodeIDs;

	// 가상 2D 그리드 상의 좌표 (뭉침 방지 연산용)
	UPROPERTY(BlueprintReadOnly)
	FIntPoint GridPosition = FIntPoint::ZeroValue;

	// 실제 월드에 스폰될 절대 좌표 (방 사이의 간격 계산 적용)
	UPROPERTY(BlueprintReadOnly)
	FVector SpawnLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsCleared = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsVisited = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsTreasureUnlocked = false;

	UPROPERTY(BlueprintReadWrite, Category = "Minimap")
	ER1MinimapRoomState MinimapState = ER1MinimapRoomState::Hidden;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMapGeneratedSignature, const TArray<struct FR1MapNode>&, MapData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerMovedRoomSignature, int32, NewRoomNodeID, int32, PrevRoomNodeID);

UCLASS()
class R1_API AR1MapGenerator : public AActor
{
	GENERATED_BODY()

public:
	AR1MapGenerator();

protected:
	virtual void BeginPlay() override;

public:
	// 생성할 전체 방의 개수
	UPROPERTY(EditAnywhere, Category = "Map Generation")
	int32 TotalRoomCount = 7;

	// 방 맵 간의 물리적 거리 (예: 10000 = 100m, 서로 보이지 않게 띄움)
	UPROPERTY(EditAnywhere, Category = "Map Generation")
	float RoomSpacing = 5000.0f;

	// 최종적으로 완성된 맵의 데이터 배열
	UPROPERTY(BlueprintReadOnly, Category = "Map Generation")
	TArray<FR1MapNode> GeneratedMap;

	UPROPERTY()
	TMap<int32, class ADungeonManager*> ActiveManagers;

	UPROPERTY()
	TSet<int32> InitializedNodeIDs;

	// 아이작 방식(가지치기) 맵 생성 메인 함수
	UFUNCTION(BlueprintCallable, Category = "Map Generation")
	void GenerateMap();

	// [추가] 에디터에서 UR1AssetData(예: DA_AssetData) 딱 하나만 넣어줍니다.
	UPROPERTY(EditAnywhere, Category = "Map Generation")
	TObjectPtr<UR1AssetData> GlobalAssetData;

public:
	UPROPERTY(BlueprintAssignable, Category = "Map Generation")
	FOnMapGeneratedSignature OnMapGenerated;

	UPROPERTY(BlueprintAssignable, Category = "Map Generation")
	FOnPlayerMovedRoomSignature OnPlayerMovedRoom;

	// 특정 방향(동서남북)에 논리적으로 연결된 방이 있는지 찾아주는 헬퍼 함수
	int32 GetConnectedNodeInDirection(int32 CurrentNodeID, ER1DoorDirection Direction);
	int32 GetCurrentNodeID() const { return CurrentActiveNodeID; }

public:
	// 세이브 데이터로부터 맵을 복원하는 함수
	UFUNCTION(BlueprintCallable, Category = "Map Generation")
	void LoadMapFromSaveData(const TArray<struct FR1MapNodeSaveData>& SavedNodes, int32 SavedFloorIndex, int32 SavedActiveNodeID, FVector SavedLocation = FVector::ZeroVector, FRotator SavedRotation = FRotator::ZeroRotator);

private:
	// 라벨 이름으로 로드된 풀(Pool)에서 방 데이터를 찾아주는 도우미 함수
	class UR1RoomDefinitionData* FindRoomDefinitionByLabel(FName Label);

public:
	UFUNCTION()
	void RegisterRoomManager(class ADungeonManager* Manager, int32 RoomNodeID = -1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Generation")
	TArray<FFloorData> FloorSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Generation")
	int32 CurrentFloorIndex = 0;

	UFUNCTION(BlueprintCallable, Category = "Map Generation")
	void GoToNextFloor();

	UFUNCTION(BlueprintPure, Category = "Map Generation")
	bool IsLastFloor() const;

	// 전체 층의 모든 방을 스폰하고, 전부 로드되면 OnFloorFullyLoaded를 호출합니다.
	void SpawnFloorAndWait();

	// 방 진입 시 명시적으로 호출: 전투 시작/문 잠금/플레이어 텔레포트.
	void ActivateRoom(int32 NodeID);

private:
	void UpdateMinimapState(int32 TargetNodeID, int32 PrevNodeID);

	// 각 룸 레벨 인스턴스의 OnLevelShown에 바인딩되는 카운터 콜백.
	// (OnLevelShown은 AddToWorld/BeginPlay 이후에 브로드캐스트되므로, 카운트가
	//  끝난 시점엔 모든 방의 DungeonManager가 ActiveManagers에 등록되어 있다.)
	UFUNCTION()
	void HandleFloorRoomShown();

	// 모든 방 로드가 끝났을 때 1회 실행: 진행도 100%, 시작/복귀 방 활성화, 로딩 게이트 해제.
	void OnFloorFullyLoaded();

	// 층 전환 시, 영속 월드에 스폰된 아이템/골드/몬스터 액터를 일괄 파괴하고 풀을 초기화.
	void CleanupFloorActors();

	// NodeID 방의 SpawnLocation에 가장 가까운 플레이어 스폰 마커를 찾습니다.
	AR1PlayerSpawnMarker* FindSpawnMarkerForNode(int32 NodeID) const;

	// 층 로딩 진행 카운터
	int32 ExpectedFloorRoomCount = 0;
	int32 LoadedFloorRoomCount = 0;
	bool bFloorActivated = false;

	// 로딩 완료 후 활성화할 방(신규 층=0, 세이브 복귀=저장된 방).
	int32 PendingActivateNodeID = 0;

private:
	// 내부적으로 알아서 채워 쓸 풀 (에디터 노출 안 함, 임시 보관용 Transient)
	UPROPERTY()
	TArray<UR1RoomDefinitionData*> StartRoomPool;

	UPROPERTY()
	TArray<UR1RoomDefinitionData*> CombatRoomPool;

	UPROPERTY()
	TArray<UR1RoomDefinitionData*> BossRoomPool;

	UPROPERTY()
	TArray<UR1RoomDefinitionData*> TreasureRoomPool;

	UPROPERTY()
	TArray<UR1RoomDefinitionData*> ShopRoomPool;

	UPROPERTY()
	TArray<UR1RoomDefinitionData*> RefreshRoomPool;

	void InitializeRoomPools();

private:
	// 생성된 맵에 방 속성(보스 방, 시작 방 등)을 할당하는 함수
	void AssignRoomTypes();

	// 해당 그리드 좌표에 이미 방이 존재하는지 확인하는 헬퍼 함수
	bool HasRoomAt(FIntPoint Pos);


	// 현재 플레이어가 위치한 방의 고유 번호 (시작은 0번)
public:

	int32 CurrentActiveNodeID = 0;

private:
	// 플레이어가 문을 밟았을 때 문의 델리게이트가 호출할 콜백 함수
	UFUNCTION()
	void OnPlayerEnteredDoor(ER1DoorDirection Direction);

public:
	int32 PendingNodeID = -1;

private:

	// [추가] 도착지 방에서 '반대편 문'을 찾기 위해, 플레이어가 밟았던 문의 방향을 기억해둡니다.
	ER1DoorDirection PendingDoorDirection = ER1DoorDirection::None;

	// [추가] 동->서, 남->북 등 반대 방향을 계산해주는 헬퍼 함수
	ER1DoorDirection GetOppositeDirection(ER1DoorDirection InDir);

private:
	// 지휘관의 무전을 수신할 콜백 함수
	UFUNCTION()
	void OnRoomClearedCallback(int32 ClearedNodeID);

private:
	// 특정 좌표에 방이 있다면 그 방의 NodeID를 반환하는 함수
	int32 GetNodeIDAt(FIntPoint Pos);

	// 풀(Pool)에서 '우리가 원하는 방향의 문'을 가진 방을 찾아 영구적으로 빼오는 함수
	class UR1RoomDefinitionData* PopValidRoomFromPool(TArray<class UR1RoomDefinitionData*>& Pool, ER1DoorDirection RequiredDoor);

public:
	// 세이브 서브시스템을 불러와 현재 상태를 조용히 저장하는 헬퍼 함수
	void TriggerAutoSave();

private:
	UFUNCTION()
	void InitializeMap();

public:
	// 🌟 2. 외부에서 구독할 수 있는 방송국 안테나를 설치합니다.
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMapGenerateProgress OnGenerateProgressUpdated;

protected:
	// 뒤로 가는 현상을 막기 위한 변수
	float HighestAchievedProgress = 0.0f;

private:
	// [Added] 세이브에서 불러온 위치 정보 보관용
	bool bIsLoadingFromSave = false;
	FVector LoadedPlayerLocation = FVector::ZeroVector;
	FRotator LoadedPlayerRotation = FRotator::ZeroRotator;

public:
	// 위젯 클래스 하나만 에디터에서 받도록 남겨둡니다. (UI 인스턴스 변수는 삭제!)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UR1LoadingScreenWidget> LoadingWidgetClass;
};