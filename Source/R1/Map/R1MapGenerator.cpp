#include "Map/R1MapGenerator.h"
#include "R1LogChannels.h"
#include "Map/DungeonManager.h"
#include "Map/R1Door.h"
#include "Map/R1MapGrid.h"
#include "Map/R1MapLayoutGenerator.h"
#include "Map/R1MinimapState.h"

#include "Data/R1RoomDefinitionData.h"
#include "System/R1RoomStreamingSubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "System/R1AssetManager.h"
#include "System/R1PlayerSaveGame.h"
#include "System/R1SaveSystem.h"
#include "Character/R1Player.h"

#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

#include "Containers/Queue.h"
#include "Data/R1AssetData.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"

#include "Player/R1PlayerController.h"
#include "System/R1LoadingSubSystem.h"
#include "UI/System/R1LoadingScreenWidget.h"
#include "Map/R1PlayerSpawnMarker.h"

#include "Object/R1ItemActor.h"
#include "Object/R1GoldActor.h"
#include "Character/R1Monster.h"
#include "System/R1ObjectPoolSystem.h"

// GenerateMap(동기) 직후 도달하는 진행도. 이후 0.2~1.0 구간을 실제 방 스트리밍 비율로 채운다.
// 느린 작업(방 AddToWorld)이 로딩바의 대부분(80%)을 차지하도록 해 50%에서 멈춘 듯한 인상을 없앤다.
static constexpr float FloorLoadStartProgress = 0.2f;

// 네비메시 생성 대기 파라미터.
static constexpr float NavBuildPollInterval = 0.05f;   // 폴링 간격(초)
static constexpr int32 NavBuildMaxTicks = 100;         // 타임아웃(약 5초) — 무한 대기 방지
static constexpr int32 NavRenotifyIntervalTicks = 20;  // navmesh 미생성 방 재통지(자가 치유) 간격(약 1초)

AR1MapGenerator::AR1MapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AR1MapGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (UR1LoadingSubSystem* LoadingSubsystem = GetGameInstance()->GetSubsystem<UR1LoadingSubSystem>())
	{
		LoadingSubsystem->ShowLoadingScreen(LoadingWidgetClass, this);
	}

	// 델리게이트 방식으로 다음 틱에 실행되도록 예약합니다.
	// 이렇게 하면 이번 프레임의 마지막에 슬레이트(UI)가 화면에 그려질 기회를 얻게 됩니다.
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &AR1MapGenerator::InitializeMap));
}

void AR1MapGenerator::InitializeMap()
{
	UR1SaveSystem* SaveSystem = GetGameInstance()->GetSubsystem<UR1SaveSystem>();

	if (SaveSystem && SaveSystem->HasSavedRun())
	{
		UE_LOG(LogR1, Warning, TEXT("[MapGenerator] 세이브 파일이 존재합니다. 랜덤 맵 생성을 대기하고 로드를 요청합니다."));

		// 플레이어 캐릭터를 찾아옴
		AR1Player* PlayerChar = Cast<AR1Player>(UGameplayStatics::GetPlayerCharacter(this, 0));

		// 서브시스템에게 맵과 스탯을 복구하라고 지시!
		SaveSystem->LoadCurrentRun(PlayerChar, this);
	}
	else
	{
		HighestAchievedProgress = 0.1f;
		OnGenerateProgressUpdated.Broadcast(HighestAchievedProgress);

		InitializeRoomPools();
		GenerateMap();

		HighestAchievedProgress = FloorLoadStartProgress;
		OnGenerateProgressUpdated.Broadcast(HighestAchievedProgress);

		PendingActivateNodeID = 0;
		SpawnFloorAndWait();
	}
}

void AR1MapGenerator::GenerateMap()
{
	// 토폴로지 생성 알고리즘은 FR1MapLayoutGenerator로 분리(C-1.2). 액터는 풀 로딩·재시도·
	// 특수방 배정·오토세이브만 담당한다. RNG 순서는 분리 전과 동일하게 보존된다.
	const FR1MapLayoutGenerator LayoutGenerator(TotalRoomCount, RoomSpacing);

	int32 MaxRetries = 50;
	bool bMapGeneratedSuccessfully = false;

	while (MaxRetries > 0 && !bMapGeneratedSuccessfully)
	{
		// 재시도할 때마다 풀이 리셋되어야 하므로 다시 로드합니다.
		InitializeRoomPools();
		ActiveManagers.Empty();
		InitializedNodeIDs.Empty();

		bMapGeneratedSuccessfully = LayoutGenerator.BuildAttempt(StartRoomPool, CombatRoomPool, GeneratedMap);

		MaxRetries--;
	}

	// 3. 루프 종료 후, 가장 멀리 있는 방을 보스 방으로 교체
	if (bMapGeneratedSuccessfully)
	{
		AssignRoomTypes();
		UE_LOG(LogR1, Warning, TEXT("[MapGenerator] 합리적인 퍼즐 맞추기로 %d개의 방 지도 생성이 완료되었습니다!"), GeneratedMap.Num());
	}
	else
	{
		UE_LOG(LogR1, Error, TEXT("[MapGenerator] 퍼즐 조각이 부족하거나 알고리즘 한계로 생성 실패!"));
	}

	TriggerAutoSave();
}

void AR1MapGenerator::InitializeRoomPools()
{
	if (!GlobalAssetData)
	{
		return;
	}

	// 에디터에서 세팅한 층 배열을 벗어나면 중단
	if (!FloorSettings.IsValidIndex(CurrentFloorIndex))
	{
		UE_LOG(LogR1, Error, TEXT("[MapGenerator] %d층 세팅 데이터가 없습니다!"), CurrentFloorIndex);
		return;
	}

	const FFloorData& CurrentFloor = FloorSettings[CurrentFloorIndex];

	// 람다(Lambda) 함수를 활용해 라벨 이름만 주면 알아서 풀을 채우도록 만듭니다.
	auto LoadPoolByLabel = [&](FName Label, TArray<UR1RoomDefinitionData*>& OutPool)
		{
			OutPool.Empty();

			const FAssetSet& AssetSet = GlobalAssetData->GetAssetSetByLabel(Label);

			// 2. 긁어온 Soft 경로들을 순회하며 메모리에 로드(TryLoad)합니다.
			for (const FAssetEntry& Entry : AssetSet.AssetEntries)
			{
				// 주의: 여기서 로드하는 것은 '무거운 3D 맵'이 아니라 '가벼운 PDA 명세서'입니다.
				// 텍스트 데이터에 불과하므로 동기 로드(TryLoad)를 해도 프레임 드랍이 전혀 없습니다!
				UObject* LoadedObj = Entry.AssetPath.TryLoad();

				// 3. 로드된 오브젝트가 방 데이터(UR1RoomDefinitionData)가 맞다면 배열에 넣습니다.
				if (UR1RoomDefinitionData* RoomData = Cast<UR1RoomDefinitionData>(LoadedObj))
				{
					OutPool.Add(RoomData);
				}
			}
		};

	LoadPoolByLabel(CurrentFloor.StartRoomLabel, StartRoomPool);
	LoadPoolByLabel(CurrentFloor.CombatRoomLabel, CombatRoomPool);
	LoadPoolByLabel(CurrentFloor.BossRoomLabel, BossRoomPool);
	LoadPoolByLabel(CurrentFloor.TreasureRoomLabel, TreasureRoomPool);
	LoadPoolByLabel(CurrentFloor.ShopRoomLabel, ShopRoomPool);
	LoadPoolByLabel(CurrentFloor.RefreshRoomLabel, RefreshRoomPool);
}

void AR1MapGenerator::AssignRoomTypes()
{
	if (BossRoomPool.IsEmpty())
	{
		return;
	}

	// 2. 보스 방 위치 찾기 (가장 먼 막다른 길)
	int32 BossNodeID = -1;
	float MaxDistance = -1.0f;
	TArray<int32> DeadEndNodes;


	for (int32 i = 1; i < GeneratedMap.Num(); ++i)
	{
		if (GeneratedMap[i].ConnectedNodeIDs.Num() == 1)
		{
			DeadEndNodes.Add(i);

			float Dist = FVector::Dist(FVector::ZeroVector, FVector(GeneratedMap[i].GridPosition.X, GeneratedMap[i].GridPosition.Y, 0));
			if (Dist > MaxDistance)
			{
				MaxDistance = Dist;
				BossNodeID = i;
			}
		}
	}

	// 3. 보스 방 할당
	if (BossNodeID != -1)
	{
		GeneratedMap[BossNodeID].RoomDefinition = BossRoomPool[FMath::RandRange(0, BossRoomPool.Num() - 1)];
		DeadEndNodes.RemoveSingle(BossNodeID);
	}

	TArray<ER1RoomContentType> AvailableSpecialTypes;

	// 각 풀이 비어있지 않다면, "이 방 타입은 이번 층에 등장할 자격이 있음"을 배열에 등록
	if (!TreasureRoomPool.IsEmpty()) AvailableSpecialTypes.Add(ER1RoomContentType::Treasure);
	if (!ShopRoomPool.IsEmpty())     AvailableSpecialTypes.Add(ER1RoomContentType::Shop);
	if (!RefreshRoomPool.IsEmpty())  AvailableSpecialTypes.Add(ER1RoomContentType::Refresh);

	TArray<UR1RoomDefinitionData*> SpecialRoomsToSpawn;

	if (AvailableSpecialTypes.Num() > 0)
	{
		// 2. 등장 자격을 얻은 방 타입들을 무작위로 섞습니다. (Shuffle)
		for (int32 i = AvailableSpecialTypes.Num() - 1; i > 0; i--)
		{
			AvailableSpecialTypes.Swap(i, FMath::RandRange(0, i));
		}

		// 3. 이번 층에 등장할 특수 방의 개수를 정합니다 (1~3개)
		int32 NumSpecialRoomsToSpawn = FMath::RandRange(1, 3);
		NumSpecialRoomsToSpawn = FMath::Min(NumSpecialRoomsToSpawn, AvailableSpecialTypes.Num());


		UE_LOG(LogR1, Warning, TEXT("[MapGenerator] 🎲 랜덤 특수 방 %d개 스폰 결정!"), NumSpecialRoomsToSpawn);


		// 타입 -> 풀 매핑. 분배 시 if-체인 대신 테이블 조회로 풀을 선택한다.
		TMap<ER1RoomContentType, TArray<UR1RoomDefinitionData*>*> SpecialPoolByType;
		SpecialPoolByType.Add(ER1RoomContentType::Treasure, &TreasureRoomPool);
		SpecialPoolByType.Add(ER1RoomContentType::Shop, &ShopRoomPool);
		SpecialPoolByType.Add(ER1RoomContentType::Refresh, &RefreshRoomPool);

		// 4. 섞인 순서대로 결정된 개수만큼 풀에서 방을 딱 하나씩만 빼옵니다.
		for (int32 i = 0; i < NumSpecialRoomsToSpawn; ++i)
		{
			ER1RoomContentType SelectedType = AvailableSpecialTypes[i];

			if (TArray<UR1RoomDefinitionData*>** PoolPtr = SpecialPoolByType.Find(SelectedType))
			{
				if (UR1RoomDefinitionData* PickedRoom = PopRandomFromPool(**PoolPtr))
				{
					SpecialRoomsToSpawn.Add(PickedRoom);
				}
			}

			switch (SelectedType)
			{
			case ER1RoomContentType::Treasure: UE_LOG(LogR1, Warning, TEXT(" - 보물 방 당첨!")); break;
			case ER1RoomContentType::Shop:     UE_LOG(LogR1, Warning, TEXT(" - 상점 방 당첨!")); break;
			case ER1RoomContentType::Refresh:  UE_LOG(LogR1, Warning, TEXT(" - 회복 방 당첨!")); break;
			default: break;
			}
		}
	}
	else
	{
		UE_LOG(LogR1, Error, TEXT("[MapGenerator] ❌ 사용 가능한 특수 방 풀이 전부 0개입니다!"));
	}

	for (UR1RoomDefinitionData* SpecialRoomData : SpecialRoomsToSpawn)
	{
		int32 TargetNodeID = -1;

		// 1순위: 남은 '막다른 길(DeadEnd)'에 우선적으로 숨겨둡니다.
		if (DeadEndNodes.Num() > 0)
		{
			int32 RandDeadEndIdx = FMath::RandRange(0, DeadEndNodes.Num() - 1);
			TargetNodeID = DeadEndNodes[RandDeadEndIdx];
			DeadEndNodes.RemoveAt(RandDeadEndIdx);
		}
		// 2순위: 막다른 길이 부족하면, 일반 방 빈자리에 배치
		else
		{
			TArray<int32> AvailableNormalNodes;
			for (int32 i = 1; i < GeneratedMap.Num(); ++i)
			{
				if (GeneratedMap[i].RoomDefinition == nullptr) AvailableNormalNodes.Add(i);
			}

			if (AvailableNormalNodes.Num() > 0)
			{
				int32 RandNormalIdx = FMath::RandRange(0, AvailableNormalNodes.Num() - 1);
				TargetNodeID = AvailableNormalNodes[RandNormalIdx];
			}
		}

		// 6. 데이터 주입 및 강제 클리어 처리
		if (TargetNodeID != -1)
		{
			GeneratedMap[TargetNodeID].RoomDefinition = SpecialRoomData;

			// 방 타입이 전투가 없는 타입이라면 무조건 클리어 상태로 만듦
			if (SpecialRoomData->RoomType == ER1RoomContentType::Treasure ||
				SpecialRoomData->RoomType == ER1RoomContentType::Shop ||
				SpecialRoomData->RoomType == ER1RoomContentType::Refresh)
			{
				GeneratedMap[TargetNodeID].bIsCleared = true;
			}
		}
	}
	// 4. 나머지 일반(Combat) 방 할당 (중복 방지 로직 적용)
	for (int32 i = 1; i < GeneratedMap.Num(); ++i)
	{

		if (GeneratedMap[i].RoomDefinition != nullptr) continue;

		// [안전장치] 풀이 비어있는지 체크 (맵의 방 개수보다 만들어둔 PDA가 적을 경우)
		if (CombatRoomPool.IsEmpty())
		{
			break;
		}

		// 랜덤으로 방 하나 뽑기
		int32 RandomIndex = FMath::RandRange(0, CombatRoomPool.Num() - 1);
		GeneratedMap[i].RoomDefinition = CombatRoomPool[RandomIndex];

		// [핵심 변경] 한 번 배정한 방은 풀에서 아예 삭제하여 중복 생성을 원천 차단합니다!
		CombatRoomPool.RemoveAt(RandomIndex);
	}
}

int32 AR1MapGenerator::GetConnectedNodeInDirection(int32 CurrentNodeID, ER1DoorDirection Direction)
{
	if (!GeneratedMap.IsValidIndex(CurrentNodeID)) return -1;

	const FR1MapNode& CurrentNode = GeneratedMap[CurrentNodeID];

	// 1. 타겟 방향의 가상 그리드 좌표를 계산합니다. (None은 유효한 이웃이 없으므로 조기 반환)
	if (Direction == ER1DoorDirection::None) return -1;
	const FIntPoint TargetGridPos = CurrentNode.GridPosition + R1MapGrid::GetGridOffset(Direction);

	// 2. 현재 방과 "연결된(Connected)" 방들 중에서, 타겟 좌표에 위치한 방이 있는지 검사합니다.
	for (int32 ConnectedID : CurrentNode.ConnectedNodeIDs)
	{
		if (GeneratedMap.IsValidIndex(ConnectedID) && GeneratedMap[ConnectedID].GridPosition == TargetGridPos)
		{
			return ConnectedID; // 찾았습니다! 그 방의 번호를 반환합니다.
		}
	}

	return -1; // 해당 방향으로 뚫린 방이 없습니다.
}

void AR1MapGenerator::OnPlayerEnteredDoor(ER1DoorDirection Direction)
{
	int32 NextNodeID = GetConnectedNodeInDirection(CurrentActiveNodeID, Direction);
	if (NextNodeID == -1) return;
	if (PendingNodeID != -1) return;

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


void AR1MapGenerator::LoadMapFromSaveData(const TArray<FR1MapNodeSaveData>& SavedNodes, int32 SavedFloorIndex, int32 SavedActiveNodeID, FVector SavedLocation, FRotator SavedRotation)
{
	CurrentFloorIndex = SavedFloorIndex;
	CurrentActiveNodeID = SavedActiveNodeID;
	PendingNodeID = -1; // 불러올 땐 이동 중이 아님을 명시

	// [Added] 위치 정보 저장
	bIsLoadingFromSave = true;
	LoadedPlayerLocation = SavedLocation;
	LoadedPlayerRotation = SavedRotation;

	// 2. 맵 데이터 뼈대 완전 초기화
	GeneratedMap.Empty();
	ActiveManagers.Empty();
	InitializedNodeIDs.Empty();

	// 3. 현재 층수에 맞춰서 PDA로부터 풀(Pool) 로딩!
	InitializeRoomPools();

	// 4. 저장된 배열을 바탕으로 맵 데이터 재조립
	for (const FR1MapNodeSaveData& SaveNode : SavedNodes)
	{
		FR1MapNode NewNode;
		NewNode.NodeID = SaveNode.NodeID;
		NewNode.bIsCleared = SaveNode.bIsCleared;
		NewNode.bIsVisited = SaveNode.bIsVisited;
		NewNode.MinimapState = SaveNode.MinimapState;
		NewNode.GridPosition = SaveNode.GridPosition;
		NewNode.ConnectedNodeIDs = SaveNode.ConnectedNodeIDs;

		NewNode.RoomDefinition = FindRoomDefinitionByLabel(SaveNode.RoomAssetName);

		NewNode.SpawnLocation = FVector(
			SaveNode.GridPosition.X * RoomSpacing,
			SaveNode.GridPosition.Y * RoomSpacing,
			0.0f
		);

		GeneratedMap.Add(NewNode);
	}

	// 5. 층 전체를 스폰하고, 모두 로드되면 저장된 방을 활성화 (위치는 LoadedPlayerLocation 사용)
	PendingActivateNodeID = CurrentActiveNodeID;
	SpawnFloorAndWait();

	UE_LOG(LogR1, Warning, TEXT("[MapGenerator] 📂 %d층 %d번 방에서 이어서 시작합니다!"), CurrentFloorIndex + 1, CurrentActiveNodeID);
}

UR1RoomDefinitionData* AR1MapGenerator::FindRoomDefinitionByLabel(FName AssetName)
{
	if (AssetName.IsNone()) return nullptr;

	// 5개의 풀을 전부 뒤져서 라벨 이름이 똑같은 방 데이터를 찾아냅니다.
	auto SearchPool = [&](const TArray<UR1RoomDefinitionData*>& Pool) -> UR1RoomDefinitionData*
		{
			for (UR1RoomDefinitionData* Data : Pool)
			{
				if (Data && Data->GetFName() == AssetName) return Data;
			}
			return nullptr;
		};

	if (auto* Found = SearchPool(StartRoomPool)) return Found;
	if (auto* Found = SearchPool(CombatRoomPool)) return Found;
	if (auto* Found = SearchPool(BossRoomPool)) return Found;
	if (auto* Found = SearchPool(TreasureRoomPool)) return Found;
	if (auto* Found = SearchPool(ShopRoomPool)) return Found;
	if (auto* Found = SearchPool(RefreshRoomPool)) return Found;

	return nullptr;
}

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

void AR1MapGenerator::ActivateRoom(int32 NodeID)
{
	if (!GeneratedMap.IsValidIndex(NodeID)) return;

	ADungeonManager* Manager = ActiveManagers.FindRef(NodeID);
	if (!IsValid(Manager))
	{
		UE_LOG(LogR1, Warning, TEXT("[MapGenerator] ActivateRoom: %d번 방 매니저가 아직 등록되지 않았습니다."), NodeID);
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
	// 세이브 복귀 위치는 한 번만 사용. 플레이어 포인터 유효성과 무관하게 즉시 소비(리셋)한다.
	const bool bWasLoadingFromSave = bIsLoadingFromSave;
	bIsLoadingFromSave = false;

	// 텔레포트 보정값: 문 앞 진입 시 방 중심 쪽으로 밀어 넣을 거리, 그리고 바닥 끼임 방지용 Z 띄움.
	constexpr float DoorEntryPushDistance = 300.0f; // 문에서 방 중심 방향으로 들어가는 거리
	constexpr float DoorEntryZOffset = 100.0f;      // 문 진입 시 Z 띄움
	constexpr float RoomCenterZOffset = 150.0f;     // 마커도 문도 없을 때 방 중심 폴백 Z 띄움

	AR1Player* PlayerCharacter = Cast<AR1Player>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (PlayerCharacter)
	{
		FVector FinalLocation;
		FRotator FinalRotation = FRotator::ZeroRotator;

		if (bWasLoadingFromSave)
		{
			FinalLocation = LoadedPlayerLocation;
			FinalRotation = LoadedPlayerRotation;
		}
		else if (TargetDoorToSpawnAt)
		{
			const FVector DirectionToCenter =
				(GeneratedMap[NodeID].SpawnLocation - TargetDoorToSpawnAt->GetActorLocation()).GetSafeNormal();
			FinalLocation = TargetDoorToSpawnAt->GetActorLocation() + (DirectionToCenter * DoorEntryPushDistance) + FVector(0.0f, 0.0f, DoorEntryZOffset);
		}
		else if (AR1PlayerSpawnMarker* Marker = FindSpawnMarkerForNode(NodeID))
		{
			FinalLocation = Marker->GetActorLocation();
			FinalRotation = Marker->GetActorRotation();
		}
		else
		{
			FinalLocation = GeneratedMap[NodeID].SpawnLocation + FVector(0.0f, 0.0f, RoomCenterZOffset);
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

	R1MinimapState::ApplyRoomEntered(GeneratedMap, NodeID);
	if (OnPlayerMovedRoom.IsBound())
	{
		OnPlayerMovedRoom.Broadcast(NodeID, PrevRoomID);
	}

	TriggerAutoSave();
}

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
				UE_LOG(LogR1, Warning,
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

	UE_LOG(LogR1, Warning, TEXT("[MapGenerator] 층 에셋 프리로드 시작: %d개 경로"), PathsToLoad.Num());
}

void AR1MapGenerator::SpawnFloorAndWait()
{
	// 층 에셋 비동기 프리로드 시작. 아래 레벨 스트리밍(AddToWorld)과 병렬로 진행되므로
	// 정상 경우 추가 대기 시간이 거의 없다. 완료 게이트는 WaitForNavMeshThenActivate에서 처리.
	StartFloorAssetPreload();

	UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
	if (!RoomSubsystem)
	{
		OnFloorFullyLoaded();
		return;
	}

	bFloorActivated = false;
	ExpectedFloorRoomCount = 0;
	LoadedFloorRoomCount = 0;

	// 1패스: 모든 방 스폰. SpawnRoomLevel은 같은 RoomDefinition에 대해 동일한
	// 인스턴스를 캐시/반환하므로, 중복 포인터는 한 번만 카운트해야 카운터가
	// 영원히 미완료로 멈추는 것을 방지할 수 있다.
	TArray<ULevelStreamingDynamic*> UniqueLevels;
	for (const FR1MapNode& Node : GeneratedMap)
	{
		if (!Node.RoomDefinition) continue;

		ULevelStreamingDynamic* Level = RoomSubsystem->SpawnRoomLevel(
			Node.RoomDefinition, Node.SpawnLocation, FRotator::ZeroRotator);
		if (!Level) continue;

		if (UniqueLevels.Contains(Level)) continue;
		UniqueLevels.Add(Level);
		ExpectedFloorRoomCount++;
	}

	if (ExpectedFloorRoomCount == 0)
	{
		OnFloorFullyLoaded();
		return;
	}

	// 2패스: 이미 표시(visible)된 건 즉시 카운트, 나머지는 OnLevelShown에 바인딩.
	// OnLevelShown은 AddToWorld(=액터 BeginPlay) 이후 호출되므로, 완료 시점에
	// 모든 DungeonManager가 등록되어 ActivateRoom이 안전하다.
	for (ULevelStreamingDynamic* Level : UniqueLevels)
	{
		if (Level->IsLevelVisible())
		{
			LoadedFloorRoomCount++;
		}
		else
		{
			Level->OnLevelShown.RemoveDynamic(this, &AR1MapGenerator::HandleFloorRoomShown);
			Level->OnLevelShown.AddDynamic(this, &AR1MapGenerator::HandleFloorRoomShown);
		}
	}

	// 이미 표시된 방이 있다면 그 비율만큼 진행도를 즉시 반영.
	BroadcastFloorLoadProgress();

	if (LoadedFloorRoomCount >= ExpectedFloorRoomCount)
	{
		OnFloorFullyLoaded();
	}
}

void AR1MapGenerator::HandleFloorRoomShown()
{
	LoadedFloorRoomCount++;

	// 방 하나가 표시될 때마다 진행도를 갱신해 로딩바가 50%에서 멈추지 않고 계속 차오르게 한다.
	BroadcastFloorLoadProgress();

	if (!bFloorActivated && LoadedFloorRoomCount >= ExpectedFloorRoomCount)
	{
		OnFloorFullyLoaded();
	}
}

void AR1MapGenerator::BroadcastFloorLoadProgress()
{
	const float Frac = (ExpectedFloorRoomCount > 0)
		? (float)LoadedFloorRoomCount / (float)ExpectedFloorRoomCount
		: 1.0f;

	const float NewProgress = FMath::Lerp(FloorLoadStartProgress, 1.0f, FMath::Clamp(Frac, 0.0f, 1.0f));

	// HighestAchievedProgress로 단조 증가 보장(뒤로 가지 않음).
	if (NewProgress > HighestAchievedProgress)
	{
		HighestAchievedProgress = NewProgress;
		OnGenerateProgressUpdated.Broadcast(HighestAchievedProgress);
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

	// 방 지오메트리는 월드에 추가됐지만 네비메시 타일은 아직 비동기 빌드 중일 수 있다.
	// 로딩 게이트(NotifyContentReady)와 방 활성화(ActivateRoom)는 네비메시가 준비된 뒤로 미룬다.
	NavBuildWaitTicks = 0;
	WaitForNavMeshThenActivate();
}

void AR1MapGenerator::WaitForNavMeshThenActivate()
{
	UWorld* World = GetWorld();
	UNavigationSystemV1* NavSys = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;

	++NavBuildWaitTicks;

	// 첫 진입 시 1회: 스트리밍된 모든 NavMeshBoundsVolume를 재통지해 첫 틱 경합을 보정.
	if (NavSys != nullptr && NavBuildWaitTicks == 1)
	{
		RenotifyAllNavBounds(NavSys);
	}

	const bool bTimedOut = (NavBuildWaitTicks >= NavBuildMaxTicks);

	// '층의 모든 방'의 navmesh를 직접 검증해 아직 길찾기 불가한 방을 수집.
	TArray<int32> UnnavigableRooms = (NavSys != nullptr) ? CollectUnnavigableRooms(NavSys) : TArray<int32>();

	if (NavSys != nullptr && !bTimedOut)
	{
		// 빌드가 진행 중(타일 일부만 완성)이면 계속 대기.
		const bool bStillBuilding = UNavigationSystemV1::IsNavigationBeingBuilt(World);

		// 에셋 프리로드 완료 여부도 같은 게이트에서 함께 기다린다.
		// 핸들이 없으면(로드할 게 없었으면) 완료로 취급한다.
		const bool bPreloadReady = (!FloorPreloadHandle.IsValid()) || FloorPreloadHandle->HasLoadCompleted();

		if (UnnavigableRooms.Num() > 0 || bStillBuilding || !bPreloadReady)
		{
			// [자가 치유] 빌드가 idle인데도 navmesh가 없는 방은 dirty 통지가 유실된 것이다.
			// 첫 틱의 전체 재통지가 처리될 시간을 주기 위해 약 1초 간격으로만 시도한다.
			if (UnnavigableRooms.Num() > 0 && !bStillBuilding && (NavBuildWaitTicks % NavRenotifyIntervalTicks == 0))
			{
				RenotifyUnnavigableRooms(NavSys, UnnavigableRooms);
			}

			World->GetTimerManager().SetTimer(
				NavBuildWaitTimer, this, &AR1MapGenerator::WaitForNavMeshThenActivate, NavBuildPollInterval, false);
			return;
		}
	}

	if (bTimedOut)
	{
		const FString RoomList = FString::JoinBy(UnnavigableRooms, TEXT(", "),
			[](int32 NodeID) { return FString::FromInt(NodeID); });
		UE_LOG(LogR1, Error,
			TEXT("[MapGenerator] navmesh 대기 타임아웃(%.1fs) — 다음 방에 navmesh가 생성되지 않았습니다: [%s]. ")
			TEXT("해당 방 레벨의 NavMeshBoundsVolume가 바닥(방 중심 포함)을 감싸는지 확인하세요."),
			NavBuildMaxTicks * NavBuildPollInterval, *RoomList);
	}

	if (bTimedOut && FloorPreloadHandle.IsValid() && !FloorPreloadHandle->HasLoadCompleted())
	{
		UE_LOG(LogR1, Warning,
			TEXT("[MapGenerator] 에셋 프리로드가 끝나기 전에 타임아웃 — 프리로드를 기다리지 않고 진행합니다."));
	}

	FinalizeFloorActivation();
}

void AR1MapGenerator::RenotifyAllNavBounds(UNavigationSystemV1* NavSys)
{
	UWorld* World = GetWorld();
	if (!NavSys || !World) return;

	// 층 전체가 동시에 대량 로드될 때 일부 룸(특히 시작 방)의 bounds 업데이트 알림이 누락되어
	// 그 방의 navmesh가 아예 생성되지 않는 경합을 보정한다. 이미 정상 등록된 volume이라도
	// 재통지는 해당 영역을 다시 더럽혀(rebuild) 무해하다.
	for (TActorIterator<ANavMeshBoundsVolume> It(World); It; ++It)
	{
		NavSys->OnNavigationBoundsUpdated(*It);
	}
}

TArray<int32> AR1MapGenerator::CollectUnnavigableRooms(UNavigationSystemV1* NavSys) const
{
	// 재통지(OnNavigationBoundsUpdated)는 PendingNavBoundsUpdates 큐에 쌓였다가 나브 시스템의 다음
	// Tick에야 처리되며, IsNavigationBeingBuilt는 이 큐를 보지 못한다(HasDirtyAreasQueued/빌드 태스크만
	// 검사). 그래서 활성화할 방만 검사하면 통지가 유실된 다른 방이 navmesh 없이 시작될 수 있다.
	// 방 중심을 navmesh에 투영해 성공하면 그 방의 타일이 생성된 것.
	// (RoomSpacing(기본 5000) 대비 충분히 작은 extent라 옆 방 navmesh로 오인하지 않는다.)
	TArray<int32> UnnavigableRooms;
	if (!NavSys) return UnnavigableRooms;

	const FVector Extent(1000.0f, 1000.0f, 1000.0f);
	for (const FR1MapNode& Node : GeneratedMap)
	{
		if (!Node.RoomDefinition) continue; // 레벨이 스폰되지 않는 노드는 검사 대상이 아님

		FNavLocation Projected;
		// 4번째 인자(NavData=nullptr)를 명시해 오버로드 모호성을 제거(기본 NavData 사용).
		if (!NavSys->ProjectPointToNavigation(Node.SpawnLocation, Projected, Extent, (const ANavigationData*)nullptr))
		{
			UnnavigableRooms.Add(Node.NodeID);
		}
	}
	return UnnavigableRooms;
}

void AR1MapGenerator::RenotifyUnnavigableRooms(UNavigationSystemV1* NavSys, const TArray<int32>& UnnavigableRooms)
{
	UWorld* World = GetWorld();
	if (!NavSys || !World) return;

	for (int32 NodeID : UnnavigableRooms)
	{
		if (!GeneratedMap.IsValidIndex(NodeID)) continue;

		const FBox RoomBox = FBox::BuildAABB(
			GeneratedMap[NodeID].SpawnLocation,
			FVector(RoomSpacing * 0.5f, RoomSpacing * 0.5f, 2000.0f));

		for (TActorIterator<ANavMeshBoundsVolume> It(World); It; ++It)
		{
			if (It->GetComponentsBoundingBox(true).Intersect(RoomBox))
			{
				NavSys->OnNavigationBoundsUpdated(*It);
			}
		}
	}

	const FString RoomList = FString::JoinBy(UnnavigableRooms, TEXT(", "),
		[](int32 NodeID) { return FString::FromInt(NodeID); });
	UE_LOG(LogR1, Warning,
		TEXT("[MapGenerator] navmesh 미생성 방 %d개 재통지(재빌드 강제): [%s]"),
		UnnavigableRooms.Num(), *RoomList);
}

void AR1MapGenerator::FinalizeFloorActivation()
{
	NavBuildWaitTicks = 0;

	// navmesh 준비 완료 → 로딩 게이트 해제 + 방 활성화.
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

void AR1MapGenerator::GoToNextFloor()
{
	CurrentFloorIndex++;

	// 마지막 층까지 깼다면 리턴 (게임 클리어)
	if (!FloorSettings.IsValidIndex(CurrentFloorIndex))
	{
		UE_LOG(LogR1, Warning, TEXT("[MapGenerator] 모든 층 클리어! 게임 엔딩!"));
		return;
	}

	if (UR1LoadingSubSystem* LoadingSubsystem = GetGameInstance()->GetSubsystem<UR1LoadingSubSystem>())
	{
		LoadingSubsystem->ShowLoadingScreen(LoadingWidgetClass, this);
	}

	// 🌟 2. 맵 생성 시작 (10% 달성 방송)
	HighestAchievedProgress = 0.1f;
	OnGenerateProgressUpdated.Broadcast(HighestAchievedProgress);

	// 0. 영속 월드에 스폰된 이전 층 액터(아이템/골드/몬스터) 일괄 정리
	CleanupFloorActors();

	// 1. 기존에 로드된 모든 스트리밍 레벨(방) 메모리에서 날려버리기
	UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
	if (RoomSubsystem)
	{
		RoomSubsystem->UnloadAllRooms();
	}

	// 2. 맵 데이터 뼈대 완전 초기화
	GeneratedMap.Empty();
	ActiveManagers.Empty();
	InitializedNodeIDs.Empty();
	CurrentActiveNodeID = 0;
	PendingNodeID = -1;
	PendingDoorDirection = ER1DoorDirection::None;

	// 3. 새로운 층 데이터 로드 및 재생성
	InitializeRoomPools();
	GenerateMap();

	HighestAchievedProgress = FloorLoadStartProgress;
	OnGenerateProgressUpdated.Broadcast(HighestAchievedProgress);

	// 4. 새 층의 모든 방을 스폰하고, 모두 로드되면 0번(시작) 방을 활성화
	PendingActivateNodeID = 0;
	SpawnFloorAndWait();
}

void AR1MapGenerator::CleanupFloorActors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	int32 DestroyedItems = 0;
	int32 DestroyedGold = 0;
	int32 DestroyedMonsters = 0;

	for (TActorIterator<AR1ItemActor> It(World); It; ++It)
	{
		if (IsValid(*It)) { It->Destroy(); ++DestroyedItems; }
	}
	for (TActorIterator<AR1GoldActor> It(World); It; ++It)
	{
		if (IsValid(*It)) { It->Destroy(); ++DestroyedGold; }
	}
	// 활성/풀링(잠자는) 몬스터 모두 월드 액터이므로 한 번에 정리
	for (TActorIterator<AR1Monster> It(World); It; ++It)
	{
		if (IsValid(*It)) { It->Destroy(); ++DestroyedMonsters; }
	}

	// 풀의 잔여 참조(파괴된 액터)를 정리
	if (UR1ObjectPoolSystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UR1ObjectPoolSystem>())
	{
		PoolSubsystem->ClearAllPools();
	}

	UE_LOG(LogR1, Warning, TEXT("[MapGenerator] 층 전환 정리: 아이템 %d, 골드 %d, 몬스터 %d 제거"), DestroyedItems, DestroyedGold, DestroyedMonsters);
}

bool AR1MapGenerator::IsLastFloor() const
{
	return CurrentFloorIndex >= (FloorSettings.Num() - 1);
}

ER1DoorDirection AR1MapGenerator::GetOppositeDirection(ER1DoorDirection InDir)
{
	return R1MapGrid::GetOppositeDirection(InDir);
}

void AR1MapGenerator::OnRoomClearedCallback(int32 ClearedNodeID)
{
	if (GeneratedMap.IsValidIndex(ClearedNodeID))
	{
		// 지도 데이터에 영구적으로 "클리어 됨" 도장을 찍습니다!
		GeneratedMap[ClearedNodeID].bIsCleared = true;

		TriggerAutoSave();

	}
}

UR1RoomDefinitionData* AR1MapGenerator::PopRandomFromPool(TArray<UR1RoomDefinitionData*>& Pool)
{
	if (Pool.IsEmpty())
	{
		return nullptr;
	}

	const int32 RandIdx = FMath::RandRange(0, Pool.Num() - 1);
	UR1RoomDefinitionData* PickedRoom = Pool[RandIdx];
	Pool.RemoveAt(RandIdx);
	return PickedRoom;
}

void AR1MapGenerator::TriggerAutoSave()
{
	if (UR1SaveSystem* SaveSystem = GetGameInstance()->GetSubsystem<UR1SaveSystem>())
	{
		AR1Player* PlayerChar = Cast<AR1Player>(UGameplayStatics::GetPlayerCharacter(this, 0));
		if (PlayerChar)
		{
			// 현재 플레이어 스탯과 맵 구조를 저장!
			int32 TempActiveID = CurrentActiveNodeID;
			if (PendingNodeID != -1)
			{
				CurrentActiveNodeID = PendingNodeID;
			}

			SaveSystem->SaveCurrentRun(PlayerChar, this);

			CurrentActiveNodeID = TempActiveID;

			UE_LOG(LogR1, Warning, TEXT("[MapGenerator] 자동 저장 완료! (현재 저장된 방: %d번)"), CurrentActiveNodeID);
		}
		else
		{
			UE_LOG(LogR1, Error, TEXT("[MapGenerator] 자동 저장 실패: 플레이어 캐릭터를 찾을 수 없습니다!"));
		}
	}
}
