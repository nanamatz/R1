

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "R1Define.h"
#include "R1MapGenerator.generated.h"

class UR1RoomDefinitionData;
class UR1AssetData;

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
	FName EventRoomLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Settings")
	FName TreasureRoomLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Settings")
	FName ShopRoomLabel;
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
	void LoadMapFromSaveData(const TArray<struct FR1MapNodeSaveData>& SavedNodes, int32 SavedFloorIndex, int32 SavedActiveNodeID);

	UFUNCTION()
	void OnSavedRoomLoaded();
private:
	// 라벨 이름으로 로드된 풀(Pool)에서 방 데이터를 찾아주는 도우미 함수
	class UR1RoomDefinitionData* FindRoomDefinitionByLabel(FName Label);

public:
	UFUNCTION()
	void RegisterRoomManager(class ADungeonManager* Manager);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Generation")
	TArray<FFloorData> FloorSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Generation")
	int32 CurrentFloorIndex = 0;

	UFUNCTION(BlueprintCallable, Category = "Map Generation")
	void GoToNextFloor();

	UFUNCTION(BlueprintPure, Category = "Map Generation")
	bool IsLastFloor() const;

private:
	void UpdateMinimapState(int32 TargetNodeID, int32 PrevNodeID);

private:
	// 내부적으로 알아서 채워 쓸 풀 (에디터 노출 안 함, 임시 보관용 Transient)
	UPROPERTY(Transient)
	TArray<UR1RoomDefinitionData*> StartRoomPool;

	UPROPERTY(Transient)
	TArray<UR1RoomDefinitionData*> CombatRoomPool;

	UPROPERTY(Transient)
	TArray<UR1RoomDefinitionData*> BossRoomPool;

	UPROPERTY(Transient)
	TArray<UR1RoomDefinitionData*> EventRoomPool;

	UPROPERTY()
	TArray<UR1RoomDefinitionData*> TreasureRoomPool;

	UPROPERTY()
	TArray<UR1RoomDefinitionData*> ShopRoomPool;

	// [추가] 맵 생성 직전에 풀을 자동으로 채우는 함수
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

	// [추가] 새 방의 로딩이 완료되었을 때 호출될 텔레포트 전용 함수
	UFUNCTION()
	void OnTransitionRoomLoaded();

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

private:
		// 세이브 서브시스템을 불러와 현재 상태를 조용히 저장하는 헬퍼 함수
		void TriggerAutoSave();
};