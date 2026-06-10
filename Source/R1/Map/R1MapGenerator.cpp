#include "Map/R1MapGenerator.h"
#include "Map/DungeonManager.h"
#include "Map/R1Door.h"

#include "Data/R1RoomDefinitionData.h"
#include "System/R1RoomStreamingSubsystem.h"
#include "System/R1PlayerSaveGame.h"
#include "System/R1SaveSystem.h"
#include "Character/R1Player.h"

#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

#include "Containers/Queue.h"
#include "Data/R1AssetData.h" 
#include "Data/R1RoomDefinitionData.h"
#include "EngineUtils.h"

#include "Player/R1PlayerController.h"
#include "System/R1LoadingSubSystem.h"
#include "UI/System/R1LoadingScreenWidget.h"
#include "Map/R1PlayerSpawnMarker.h"

#include "Object/R1ItemActor.h"
#include "Object/R1GoldActor.h"
#include "Character/R1Monster.h"
#include "System/R1ObjectPoolSystem.h"

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
		UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 세이브 파일이 존재합니다. 랜덤 맵 생성을 대기하고 로드를 요청합니다."));

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

		HighestAchievedProgress = 0.5f;
		OnGenerateProgressUpdated.Broadcast(HighestAchievedProgress);

		PendingActivateNodeID = 0;
		SpawnFloorAndWait();
	}
}

void AR1MapGenerator::GenerateMap()
{
	int32 MaxRetries = 50;
	bool bMapGeneratedSuccessfully = false;

	while (MaxRetries > 0 && !bMapGeneratedSuccessfully)
	{
		// 재시도할 때마다 풀이 리셋되어야 하므로 다시 로드합니다.
		InitializeRoomPools();
		GeneratedMap.Empty();
		ActiveManagers.Empty();
		InitializedNodeIDs.Empty();

		TQueue<int32> RoomQueue;
		int32 CurrentNodeID = 0;

		// 1. 시작 방 배정 (풀에서 1개 빼오기)
		FR1MapNode StartNode;
		StartNode.NodeID = CurrentNodeID;
		StartNode.GridPosition = FIntPoint(0, 0);
		StartNode.SpawnLocation = FVector::ZeroVector;

		if (StartRoomPool.Num() > 0)
		{
			int32 StartIndex = FMath::RandRange(0, StartRoomPool.Num() - 1);
			StartNode.RoomDefinition = StartRoomPool[StartIndex];
			StartRoomPool.RemoveAt(StartIndex);
		}
		StartNode.bIsCleared = true;

		GeneratedMap.Add(StartNode);
		RoomQueue.Enqueue(CurrentNodeID);
		CurrentNodeID++;

		// 2. [핵심] 큐 순회 (퍼즐 맞추기)
		while (!RoomQueue.IsEmpty() && CurrentNodeID < TotalRoomCount)
		{
			int32 ParentID;
			RoomQueue.Dequeue(ParentID);
			FR1MapNode& ParentNode = GeneratedMap[ParentID];

			if (!ParentNode.RoomDefinition) continue;

			// 현재 방에 실제로 뚫려있는 문 방향만 가져옵니다! (무지성 동서남북 배제)
			TArray<ER1DoorDirection> ParentDoors = ParentNode.RoomDefinition->AvailableDoors;

			// 가지가 한쪽으로만 뻗는 걸 막기 위해 문 방향 셔플
			for (int32 i = ParentDoors.Num() - 1; i > 0; i--)
			{
				ParentDoors.Swap(i, FMath::RandRange(0, i));
			}

			// 실제 뚫려있는 문을 향해서만 가지를 뻗습니다.
			for (ER1DoorDirection DoorDir : ParentDoors)
			{
				if (CurrentNodeID >= TotalRoomCount) break;

				FIntPoint DirOffset = FIntPoint::ZeroValue;
				ER1DoorDirection OppositeDir = ER1DoorDirection::None;

				switch (DoorDir)
				{
				case ER1DoorDirection::North: DirOffset = FIntPoint(1, 0);  OppositeDir = ER1DoorDirection::South; break; // 북쪽은 +X
				case ER1DoorDirection::South: DirOffset = FIntPoint(-1, 0); OppositeDir = ER1DoorDirection::North; break; // 남쪽은 -X
				case ER1DoorDirection::East:  DirOffset = FIntPoint(0, 1);  OppositeDir = ER1DoorDirection::West; break;  // 동쪽은 +Y
				case ER1DoorDirection::West:  DirOffset = FIntPoint(0, -1); OppositeDir = ER1DoorDirection::East; break;  // 서쪽은 -Y
				}

				FIntPoint NewPos = ParentNode.GridPosition + DirOffset;

				// 이미 그 위치에 다른 방이 있다면, 연결만 해주고 스킵
				if (int32 ExistingID = GetNodeIDAt(NewPos); ExistingID != -1)
				{
					GeneratedMap[ParentID].ConnectedNodeIDs.AddUnique(ExistingID);
					GeneratedMap[ExistingID].ConnectedNodeIDs.AddUnique(ParentID);
					continue;
				}

				// 뭉침 방지 룰 (아이작 룰)
				int32 NeighborCount = 0;
				FIntPoint CheckDirs[4] = { FIntPoint(0, 1), FIntPoint(0, -1), FIntPoint(-1, 0), FIntPoint(1, 0) };
				for (FIntPoint CheckDir : CheckDirs)
				{
					if (GetNodeIDAt(NewPos + CheckDir) != -1) NeighborCount++;
				}
				if (NeighborCount > 1) continue;

				// 확률 50%
				if (FMath::RandRange(0, 100) < 50)
				{
					// [가장 중요] 다음 방은 반드시 내 문과 맞물리는(OppositeDir) 문이 있어야 합니다!
					UR1RoomDefinitionData* NextRoomData = PopValidRoomFromPool(CombatRoomPool, OppositeDir);

					if (!NextRoomData)
					{
						UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 핏이 맞는 방이 풀에 고갈되었습니다. 이쪽 방향은 벽으로 막습니다."));
						continue; // 풀에 퍼즐 조각이 없으면 방을 만들지 않음
					}

					FR1MapNode NewNode;
					NewNode.NodeID = CurrentNodeID;
					NewNode.GridPosition = NewPos;
					NewNode.SpawnLocation = FVector(NewPos.X * RoomSpacing, NewPos.Y * RoomSpacing, 0.0f);
					NewNode.RoomDefinition = NextRoomData; // 바로 할당!

					NewNode.ConnectedNodeIDs.Add(ParentID);
					GeneratedMap.Add(NewNode);
					GeneratedMap[ParentID].ConnectedNodeIDs.Add(CurrentNodeID);

					RoomQueue.Enqueue(CurrentNodeID);
					CurrentNodeID++;
				}
			}
		}

		if (GeneratedMap.Num() == TotalRoomCount)
		{
			bMapGeneratedSuccessfully = true;
		}

		MaxRetries--;
	}

	// 3. 루프 종료 후, 가장 멀리 있는 방을 보스 방으로 교체
	if (bMapGeneratedSuccessfully)
	{
		AssignRoomTypes();
		UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 합리적인 퍼즐 맞추기로 %d개의 방 지도 생성이 완료되었습니다!"), GeneratedMap.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MapGenerator] 퍼즐 조각이 부족하거나 알고리즘 한계로 생성 실패!"));
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
		UE_LOG(LogTemp, Error, TEXT("[MapGenerator] %d층 세팅 데이터가 없습니다!"), CurrentFloorIndex);
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


		UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 🎲 랜덤 특수 방 %d개 스폰 결정!"), NumSpecialRoomsToSpawn);


		// 4. 섞인 순서대로 결정된 개수만큼 풀에서 방을 딱 하나씩만 빼옵니다.
		for (int32 i = 0; i < NumSpecialRoomsToSpawn; ++i)
		{
			ER1RoomContentType SelectedType = AvailableSpecialTypes[i];

			if (SelectedType == ER1RoomContentType::Treasure)
			{
				int32 RandIdx = FMath::RandRange(0, TreasureRoomPool.Num() - 1);
				SpecialRoomsToSpawn.Add(TreasureRoomPool[RandIdx]);
				TreasureRoomPool.RemoveAt(RandIdx);
			}
			else if (SelectedType == ER1RoomContentType::Shop)
			{
				int32 RandIdx = FMath::RandRange(0, ShopRoomPool.Num() - 1);
				SpecialRoomsToSpawn.Add(ShopRoomPool[RandIdx]);
				ShopRoomPool.RemoveAt(RandIdx);
			}
			else if (SelectedType == ER1RoomContentType::Refresh)
			{
				int32 RandIdx = FMath::RandRange(0, RefreshRoomPool.Num() - 1);
				SpecialRoomsToSpawn.Add(RefreshRoomPool[RandIdx]);
				RefreshRoomPool.RemoveAt(RandIdx);
			}

			if(SelectedType == ER1RoomContentType::Treasure)
			{
				UE_LOG(LogTemp, Warning, TEXT(" - 보물 방 당첨!"));
			}
			else if (SelectedType == ER1RoomContentType::Shop)
			{
				UE_LOG(LogTemp, Warning, TEXT(" - 상점 방 당첨!"));
			}
			else if (SelectedType == ER1RoomContentType::Refresh)
			{
				UE_LOG(LogTemp, Warning, TEXT(" - 회복 방 당첨!"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MapGenerator] ❌ 사용 가능한 특수 방 풀이 전부 0개입니다!"));
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

bool AR1MapGenerator::HasRoomAt(FIntPoint Pos)
{
	for (const FR1MapNode& Node : GeneratedMap)
	{
		if (Node.GridPosition == Pos) return true;
	}
	return false;
}

int32 AR1MapGenerator::GetConnectedNodeInDirection(int32 CurrentNodeID, ER1DoorDirection Direction)
{
	if (!GeneratedMap.IsValidIndex(CurrentNodeID)) return -1;

	const FR1MapNode& CurrentNode = GeneratedMap[CurrentNodeID];
	FIntPoint TargetGridPos = CurrentNode.GridPosition;

	// 1. 타겟 방향의 가상 그리드 좌표를 계산합니다.
	switch (Direction)
	{
	case ER1DoorDirection::North: TargetGridPos.X += 1; break;
	case ER1DoorDirection::South: TargetGridPos.X -= 1; break;
	case ER1DoorDirection::East:  TargetGridPos.Y += 1; break;
	case ER1DoorDirection::West:  TargetGridPos.Y -= 1; break;
	default: return -1;
	}

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

	UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 📂 %d층 %d번 방에서 이어서 시작합니다!"), CurrentFloorIndex + 1, CurrentActiveNodeID);
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
	// 세이브 복귀 위치는 한 번만 사용. 플레이어 포인터 유효성과 무관하게 즉시 소비(리셋)한다.
	const bool bWasLoadingFromSave = bIsLoadingFromSave;
	bIsLoadingFromSave = false;

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

	if (LoadedFloorRoomCount >= ExpectedFloorRoomCount)
	{
		OnFloorFullyLoaded();
	}
}

void AR1MapGenerator::HandleFloorRoomShown()
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

void AR1MapGenerator::GoToNextFloor()
{
	CurrentFloorIndex++;

	// 마지막 층까지 깼다면 리턴 (게임 클리어)
	if (!FloorSettings.IsValidIndex(CurrentFloorIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 모든 층 클리어! 게임 엔딩!"));
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

	HighestAchievedProgress = 0.5f;
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

	UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 층 전환 정리: 아이템 %d, 골드 %d, 몬스터 %d 제거"), DestroyedItems, DestroyedGold, DestroyedMonsters);
}

bool AR1MapGenerator::IsLastFloor() const
{
	return CurrentFloorIndex >= (FloorSettings.Num() - 1);
}

void AR1MapGenerator::UpdateMinimapState(int32 TargetNodeID, int32 PrevNodeID)
{
	for (FR1MapNode& Node : GeneratedMap)
	{
		if (Node.MinimapState == ER1MinimapRoomState::Current && Node.NodeID != TargetNodeID)
		{
			// 클리어 여부에 따라 상태 결정 (만약 안 깬 방에서 도망쳐 나온 거라면 Discovered로 유지)
			Node.MinimapState = Node.bIsCleared ? ER1MinimapRoomState::Visited : ER1MinimapRoomState::Discovered;
		}
	}

	// 2. 새로 진입한 방을 'Current(현재 위치)'로 변경
	if (GeneratedMap.IsValidIndex(TargetNodeID))
	{
		GeneratedMap[TargetNodeID].MinimapState = ER1MinimapRoomState::Current;
		GeneratedMap[TargetNodeID].bIsVisited = true;

		// 3. 진입한 방과 연결된 모든 이웃 방들을 탐색하여 'Hidden'이면 'Discovered(발견됨)'로 밝힙니다.
		for (int32 ConnectedID : GeneratedMap[TargetNodeID].ConnectedNodeIDs)
		{
			if (GeneratedMap.IsValidIndex(ConnectedID))
			{
				if (GeneratedMap[ConnectedID].MinimapState == ER1MinimapRoomState::Hidden)
				{
					GeneratedMap[ConnectedID].MinimapState = ER1MinimapRoomState::Discovered;
				}
			}
		}
	}
}

ER1DoorDirection AR1MapGenerator::GetOppositeDirection(ER1DoorDirection InDir)
{
	switch (InDir)
	{
	case ER1DoorDirection::North: return ER1DoorDirection::South;
	case ER1DoorDirection::South: return ER1DoorDirection::North;
	case ER1DoorDirection::East:  return ER1DoorDirection::West;
	case ER1DoorDirection::West:  return ER1DoorDirection::East;
	default: return ER1DoorDirection::None;
	}
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

int32 AR1MapGenerator::GetNodeIDAt(FIntPoint Pos)
{
	for (int32 i = 0; i < GeneratedMap.Num(); ++i)
	{
		if (GeneratedMap[i].GridPosition == Pos) return GeneratedMap[i].NodeID;
	}
	return -1;
}

UR1RoomDefinitionData* AR1MapGenerator::PopValidRoomFromPool(TArray<class UR1RoomDefinitionData*>& Pool, ER1DoorDirection RequiredDoor)
{
	// 항상 같은 방만 나오는 것을 막기 위해 인덱스를 랜덤으로 섞어서 탐색
	TArray<int32> Indices;
	for (int32 i = 0; i < Pool.Num(); ++i) Indices.Add(i);

	for (int32 i = Indices.Num() - 1; i > 0; i--)
	{
		Indices.Swap(i, FMath::RandRange(0, i));
	}

	// 섞인 순서대로 조건에 맞는 방(퍼즐 조각) 찾기
	for (int32 Index : Indices)
	{
		UR1RoomDefinitionData* RoomData = Pool[Index];
		// 이 방이 우리가 요구하는 문을 가지고 있는가?
		if (RoomData && RoomData->AvailableDoors.Contains(RequiredDoor))
		{
			Pool.RemoveAt(Index); // 중복 방지를 위해 풀에서 완전히 삭제!
			return RoomData;
		}
	}
	return nullptr; // 조건에 맞는 방이 풀에 없습니다.
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

			UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 자동 저장 완료! (현재 저장된 방: %d번)"), CurrentActiveNodeID);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[MapGenerator] 자동 저장 실패: 플레이어 캐릭터를 찾을 수 없습니다!"));
		}
	}
}
