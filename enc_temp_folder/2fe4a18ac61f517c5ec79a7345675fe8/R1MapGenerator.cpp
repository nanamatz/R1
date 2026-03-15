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
#include "EngineUtils.h"

#include "Player/R1PlayerController.h"
#include "System/R1LoadingSubSystem.h"
#include "UI/System/R1LoadingScreenWidget.h"

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
		AR1PlayerController* PC = Cast<AR1PlayerController>(UGameplayStatics::GetPlayerController(this, 0));
		if (PC)
		{
			PC->SetInputMode(FInputModeUIOnly());
			PC->bShowMouseCursor = false;
		}
	}

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

		// 0번 방 스폰 로직 (기존 코드 유지)
		UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
		if (GeneratedMap.Num() > 0 && GeneratedMap[0].RoomDefinition != nullptr && RoomSubsystem)
		{
			RoomSubsystem->SpawnRoomLevel(GeneratedMap[0].RoomDefinition, GeneratedMap[0].SpawnLocation, FRotator::ZeroRotator);
		}
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
		int32 BossNodeID = -1;
		float MaxDistance = -1.0f;

		for (int32 i = 1; i < GeneratedMap.Num(); ++i)
		{
			if (GeneratedMap[i].ConnectedNodeIDs.Num() == 1) // 막다른 길
			{
				float Dist = FVector::Dist(FVector::ZeroVector, FVector(GeneratedMap[i].GridPosition.X, GeneratedMap[i].GridPosition.Y, 0));
				if (Dist > MaxDistance)
				{
					MaxDistance = Dist;
					BossNodeID = i;
				}
			}
		}

		if (BossNodeID != -1 && BossRoomPool.Num() > 0)
		{
			// (주의: 완벽한 퍼즐을 위해선 보스 방도 문의 방향이 맞아야 합니다.
			// 지금은 심플하게 덮어씌웁니다. 보스 방 PDA는 모든 방향의 문(N,S,E,W)을 들고 있게 세팅하는 것이 안전합니다.)
			GeneratedMap[BossNodeID].RoomDefinition = BossRoomPool[FMath::RandRange(0, BossRoomPool.Num() - 1)];
		}

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
	LoadPoolByLabel(CurrentFloor.EventRoomLabel, EventRoomPool);
	LoadPoolByLabel(CurrentFloor.TreasureRoomLabel, TreasureRoomPool);
	LoadPoolByLabel(CurrentFloor.ShopRoomLabel, ShopRoomPool);
}

void AR1MapGenerator::AssignRoomTypes()
{
	if (StartRoomPool.IsEmpty() || CombatRoomPool.IsEmpty() || BossRoomPool.IsEmpty() || EventRoomPool.IsEmpty())
	{
		return;
	}

	// 1. 시작 방 할당 (0번 방)
	GeneratedMap[0].RoomDefinition = StartRoomPool[FMath::RandRange(0, StartRoomPool.Num() - 1)];
	GeneratedMap[0].bIsCleared = true;

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

	// 한 층에 등장할 이벤트 방의 개수 설정
	int32 TargetEventRoomCount = FMath::RandRange(1, 2);
	int32 SpawnedEventRooms = 0;
	TArray<UR1RoomDefinitionData*> SpecialRoomsToSpawn;

	if (!TreasureRoomPool.IsEmpty())
	{
		int32 RandIdx = FMath::RandRange(0, TreasureRoomPool.Num() - 1);
		SpecialRoomsToSpawn.Add(TreasureRoomPool[RandIdx]);
		TreasureRoomPool.RemoveAt(RandIdx); // 중복 방지를 위해 풀에서 제거
	}

	if (!ShopRoomPool.IsEmpty())
	{
		int32 RandIdx = FMath::RandRange(0, ShopRoomPool.Num() - 1);
		SpecialRoomsToSpawn.Add(ShopRoomPool[RandIdx]);
		ShopRoomPool.RemoveAt(RandIdx); // 중복 방지를 위해 풀에서 제거
	}

	for (UR1RoomDefinitionData* SpecialRoomData : SpecialRoomsToSpawn)
	{
		int32 TargetNodeID = -1;

		// 1순위: 남은 '막다른 길'에 우선적으로 숨겨둡니다.
		if (DeadEndNodes.Num() > 0)
		{
			int32 RandDeadEndIdx = FMath::RandRange(0, DeadEndNodes.Num() - 1);
			TargetNodeID = DeadEndNodes[RandDeadEndIdx];
			DeadEndNodes.RemoveAt(RandDeadEndIdx); // 쓴 자리는 제외
		}
		// 2순위: 맵 구조상 막다른 길이 더 이상 없다면, 중간에 남은 아무 빈 방에나 배치합니다.
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

		// 최종적으로 방 위치가 결정되었다면 데이터 주입!
		if (TargetNodeID != -1)
		{
			GeneratedMap[TargetNodeID].RoomDefinition = SpecialRoomData;
		}
	}

	//while (SpawnedEventRooms < TargetEventRoomCount && DeadEndNodes.Num() > 0 && !EventRoomPool.IsEmpty())
	//{
	//	// 랜덤한 막다른 길 하나 선택
	//	int32 RandomIdx = FMath::RandRange(0, DeadEndNodes.Num() - 1);
	//	int32 TargetNodeID = DeadEndNodes[RandomIdx];
	//	DeadEndNodes.RemoveAt(RandomIdx); // 중복 방지

	//	// 이벤트 풀에서 랜덤한 이벤트 방(상점/보물 등) 하나 꺼내서 할당
	//	int32 EventRoomIdx = FMath::RandRange(0, EventRoomPool.Num() - 1);
	//	GeneratedMap[TargetNodeID].RoomDefinition = EventRoomPool[EventRoomIdx];
	//	EventRoomPool.RemoveAt(EventRoomIdx);

	//	SpawnedEventRooms++;
	//}

	//// 4-2. 만약 맵 구조상 '막다른 길'이 부족하다면, 중간에 끼어있는 일반 방에라도 배치합니다.
	//if (SpawnedEventRooms < TargetEventRoomCount)
	//{
	//	TArray<int32> AvailableNormalNodes;
	//	for (int32 i = 1; i < GeneratedMap.Num(); ++i)
	//	{
	//		// 아직 어떤 방도 할당되지 않은(nullptr) 빈 방만 찾습니다.
	//		if (GeneratedMap[i].RoomDefinition == nullptr)
	//		{
	//			AvailableNormalNodes.Add(i);
	//		}
	//	}

	//	while (SpawnedEventRooms < TargetEventRoomCount && AvailableNormalNodes.Num() > 0 && !EventRoomPool.IsEmpty())
	//	{
	//		int32 RandomIdx = FMath::RandRange(0, AvailableNormalNodes.Num() - 1);
	//		int32 TargetNodeID = AvailableNormalNodes[RandomIdx];
	//		AvailableNormalNodes.RemoveAt(RandomIdx);

	//		int32 EventRoomIdx = FMath::RandRange(0, EventRoomPool.Num() - 1);
	//		GeneratedMap[TargetNodeID].RoomDefinition = EventRoomPool[EventRoomIdx];
	//		EventRoomPool.RemoveAt(EventRoomIdx);

	//		SpawnedEventRooms++;
	//	}
	//}

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

	if (NextNodeID != -1)
	{
		if (PendingNodeID != -1) return;

		PendingNodeID = NextNodeID;
		PendingDoorDirection = Direction;

		UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
		if (RoomSubsystem)
		{
			// 서브시스템에 다음 방 스폰(또는 기존 방 가져오기) 요청
			ULevelStreamingDynamic* StreamingLevel = RoomSubsystem->SpawnRoomLevel(
				GeneratedMap[NextNodeID].RoomDefinition,
				GeneratedMap[NextNodeID].SpawnLocation,
				FRotator::ZeroRotator
			);

			if (StreamingLevel)
			{
				// 방이 이미 로드되어 있다면 즉시 텔레포트 함수 호출
				if (StreamingLevel->IsLevelLoaded())
				{
					OnTransitionRoomLoaded();
				}
				// 아니라면 로딩 완료 시점에 호출되도록 델리게이트 연결
				else
				{
					StreamingLevel->OnLevelLoaded.AddDynamic(this, &AR1MapGenerator::OnTransitionRoomLoaded);
				}
			}
			else
			{
				PendingNodeID = -1;
			}
		}
	}
}


void AR1MapGenerator::LoadMapFromSaveData(const TArray<FR1MapNodeSaveData>& SavedNodes, int32 SavedFloorIndex, int32 SavedActiveNodeID)
{
	CurrentFloorIndex = SavedFloorIndex;
	CurrentActiveNodeID = SavedActiveNodeID;
	PendingNodeID = -1; // 불러올 땐 이동 중이 아님을 명시

	// 2. 맵 데이터 뼈대 완전 초기화
	GeneratedMap.Empty();

	// 3. 현재 층수에 맞춰서 PDA로부터 풀(Pool) 로딩!
	InitializeRoomPools();

	// 4. 저장된 배열을 바탕으로 맵 데이터 재조립
	for (const FR1MapNodeSaveData& SaveNode : SavedNodes)
	{
		FR1MapNode NewNode;
		NewNode.NodeID = SaveNode.NodeID;
		NewNode.bIsCleared = SaveNode.bIsCleared;
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

	// 1단계: 맵 전체를 일단 짙은 안개(Hidden)로 덮어 초기화합니다.
	for (FR1MapNode& Node : GeneratedMap)
	{
		Node.MinimapState = ER1MinimapRoomState::Hidden;
	}

	// 2단계: 플레이어가 현재 있는 방(Current)과 이미 클리어한 방(Visited)들의 도장을 확실하게 찍습니다.
	for (FR1MapNode& Node : GeneratedMap)
	{
		if (Node.NodeID == CurrentActiveNodeID)
		{
			Node.MinimapState = ER1MinimapRoomState::Current;
		}
		else if (Node.bIsCleared)
		{
			Node.MinimapState = ER1MinimapRoomState::Visited;
		}
	}

	// 3단계: 도장이 찍힌 방(Current, Visited)들을 기준으로, 연결된 모든 이웃 방의 시야를 밝힙니다(Discovered).
	for (const FR1MapNode& Node : GeneratedMap)
	{
		// 이 방이 내가 서 있거나 이미 지나온 방이라면?
		if (Node.MinimapState == ER1MinimapRoomState::Current || Node.MinimapState == ER1MinimapRoomState::Visited)
		{
			// 이 방과 연결된 모든 이웃 방들을 검사합니다.
			for (int32 ConnID : Node.ConnectedNodeIDs)
			{
				if (GeneratedMap.IsValidIndex(ConnID))
				{
					// 이웃 방이 아직 안개 속(Hidden)에 있다면, 발견됨(Discovered) 상태로 바꿔줍니다!
					if (GeneratedMap[ConnID].MinimapState == ER1MinimapRoomState::Hidden)
					{
						GeneratedMap[ConnID].MinimapState = ER1MinimapRoomState::Discovered;
					}
				}
			}
		}
	}
	// 5. 플레이어가 껐을 때 마지막으로 있던 방(CurrentActiveNodeID) 스폰!
	UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
	if (RoomSubsystem && GeneratedMap.IsValidIndex(CurrentActiveNodeID) && GeneratedMap[CurrentActiveNodeID].RoomDefinition != nullptr)
	{
		ULevelStreamingDynamic* StreamingLevel = RoomSubsystem->SpawnRoomLevel(
			GeneratedMap[CurrentActiveNodeID].RoomDefinition,
			GeneratedMap[CurrentActiveNodeID].SpawnLocation,
			FRotator::ZeroRotator
		);

		if (StreamingLevel)
		{
			// 로드 완료 시 OnSavedRoomLoaded 호출
			if (StreamingLevel->IsLevelLoaded())
			{
				OnSavedRoomLoaded();
			}
			else
			{
				StreamingLevel->OnLevelLoaded.AddDynamic(this, &AR1MapGenerator::OnSavedRoomLoaded);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 📂 %d층 %d번 방에서 이어서 시작합니다!"), CurrentFloorIndex + 1, CurrentActiveNodeID);
	}
}

void AR1MapGenerator::OnSavedRoomLoaded()
{
	// 1. 방금 비동기 로딩이 완료된 현재 방의 좌표를 가져옵니다.
	FVector SavedRoomLocation = GeneratedMap[CurrentActiveNodeID].SpawnLocation;

	// 2. 월드에 존재하는 모든 던전 매니저를 싹 뒤집니다.
	for (TActorIterator<ADungeonManager> ManagerIt(GetWorld()); ManagerIt; ++ManagerIt)
	{
		FVector2D ManagerLoc2D(ManagerIt->GetActorLocation().X, ManagerIt->GetActorLocation().Y);
		FVector2D SavedLoc2D(SavedRoomLocation.X, SavedRoomLocation.Y);

		if (FVector2D::Distance(ManagerLoc2D, SavedLoc2D) < 100.0f)
		{
			RegisterRoomManager(*ManagerIt);
			break;
		}
	}
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

	return nullptr;
}

void AR1MapGenerator::RegisterRoomManager(ADungeonManager* Manager)
{
	if (!IsValid(Manager)) return;

	// 1. 이 매니저가 현재 지도의 몇 번 방 매니저인지 '위치(Location)'로 신원 파악을 합니다.
	int32 MatchedNodeID = -1;
	for (int32 i = 0; i < GeneratedMap.Num(); ++i)
	{
		if (GeneratedMap[i].SpawnLocation.Equals(Manager->GetActorLocation(), 10.0f))
		{
			MatchedNodeID = i;
			break;
		}
	}

	if (MatchedNodeID == -1) return; // 맵에 없는 유령 방이면 무시

	AR1Door* TargetDoorToSpawnAt = nullptr;
	ER1DoorDirection OppositeDir = GetOppositeDirection(PendingDoorDirection);

	Manager->RoomNodeID = MatchedNodeID;
	Manager->OnRoomCleared.RemoveDynamic(this, &AR1MapGenerator::OnRoomClearedCallback);
	Manager->OnRoomCleared.AddDynamic(this, &AR1MapGenerator::OnRoomClearedCallback);

	for (AR1Door* Door : Manager->RoomDoors)
	{
		if (!IsValid(Door)) continue;

		int32 TargetNode = GetConnectedNodeInDirection(MatchedNodeID, Door->DoorDirection);
		Door->SetupDoorConnection(TargetNode);

		if (TargetNode != -1)
		{
			Door->OnDoorEntered.RemoveDynamic(this, &AR1MapGenerator::OnPlayerEnteredDoor);
			Door->OnDoorEntered.AddDynamic(this, &AR1MapGenerator::OnPlayerEnteredDoor);
		}

		if (Door->DoorDirection == OppositeDir)
		{
			TargetDoorToSpawnAt = Door;
		}
	}

	if (GeneratedMap[MatchedNodeID].bIsCleared)
	{
		Manager->bIsCleared = true;
		Manager->UnlockRoomDoors();
	}
	else
	{
		Manager->LockRoomDoors();
	}
	Manager->StartRoomCombat();

	//플레이어 텔레포트 및 UI 갱신
	AR1Player* PlayerCharacter = Cast<AR1Player>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (PendingNodeID == -1)
	{
		if (PlayerCharacter)
		{
			FVector SafeLocation = GeneratedMap[MatchedNodeID].SpawnLocation + FVector(0.0f, 0.0f, 150.0f);
			PlayerCharacter->SetActorLocation(SafeLocation);

			HighestAchievedProgress = 1.0f;
			OnGenerateProgressUpdated.Broadcast(HighestAchievedProgress);

			if (OnMapGenerated.IsBound())
			{
				OnMapGenerated.Broadcast(GeneratedMap);
			}
		}

		UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
		if (RoomSubsystem)
		{
			RoomSubsystem->MarkRoomGameplayReady(GeneratedMap[0].RoomDefinition);

			TArray<UR1RoomDefinitionData*> AdjacentRooms;
			for (int32 ConnectedID : GeneratedMap[MatchedNodeID].ConnectedNodeIDs)
			{
				if (GeneratedMap.IsValidIndex(ConnectedID) && GeneratedMap[ConnectedID].RoomDefinition != nullptr)
				{
					AdjacentRooms.Add(GeneratedMap[ConnectedID].RoomDefinition);
				}
			}
			RoomSubsystem->QueuePreloadRooms(AdjacentRooms);
		}

		UpdateMinimapState(MatchedNodeID, -1);

		if (OnMapGenerated.IsBound())
		{
			OnMapGenerated.Broadcast(GeneratedMap); 
		}
	}
	//문을 타고 다른 방으로 이동했을 때 세팅 
	else if (MatchedNodeID == PendingNodeID)
	{
		int32 PrevRoomID = CurrentActiveNodeID;
		CurrentActiveNodeID = MatchedNodeID;

		if (PlayerCharacter && TargetDoorToSpawnAt)
		{
			FVector DirectionToCenter = (GeneratedMap[MatchedNodeID].SpawnLocation - TargetDoorToSpawnAt->GetActorLocation()).GetSafeNormal();
			FVector SafeLocation = TargetDoorToSpawnAt->GetActorLocation() + (DirectionToCenter * 300.0f) + FVector(0.0f, 0.0f, 100.0f);

			PlayerCharacter->SetActorLocation(SafeLocation);
			PlayerCharacter->GetVelocity() = FVector::ZeroVector;

			if (AR1PlayerController* PC = Cast<AR1PlayerController>(PlayerCharacter->GetController()))
			{
				PC->ResetMovementState();
			}
		}

		UpdateMinimapState(CurrentActiveNodeID, PrevRoomID);
		if (OnPlayerMovedRoom.IsBound())
		{
			OnPlayerMovedRoom.Broadcast(CurrentActiveNodeID, PrevRoomID); // 미니맵 UI 갱신 방송!
		}

		PendingNodeID = -1;

		UR1RoomStreamingSubsystem* InnerRoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
		if (InnerRoomSubsystem)
		{
			InnerRoomSubsystem->MarkRoomGameplayReady(GeneratedMap[CurrentActiveNodeID].RoomDefinition);

			// 다음 인접 방 프리로드 (메모리 최적화)
			TArray<UR1RoomDefinitionData*> AdjacentRooms;
			for (int32 ConnectedID : GeneratedMap[CurrentActiveNodeID].ConnectedNodeIDs)
			{
				AdjacentRooms.Add(GeneratedMap[ConnectedID].RoomDefinition);
			}
			InnerRoomSubsystem->QueuePreloadRooms(AdjacentRooms);
		}
	}
	
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

	// 1. 기존에 로드된 모든 스트리밍 레벨(방) 메모리에서 날려버리기
	UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
	if (RoomSubsystem)
	{
		RoomSubsystem->UnloadAllRooms();
	}

	// 2. 맵 데이터 뼈대 완전 초기화
	GeneratedMap.Empty();
	CurrentActiveNodeID = 0;
	PendingNodeID = -1;
	PendingDoorDirection = ER1DoorDirection::None;

	// 3. 새로운 층 데이터 로드 및 재생성
	InitializeRoomPools();
	GenerateMap();

	HighestAchievedProgress = 0.5f;
	OnGenerateProgressUpdated.Broadcast(HighestAchievedProgress);

	// 4. 새로운 층의 0번 방 스폰 지시
	if (GeneratedMap.Num() > 0 && GeneratedMap[0].RoomDefinition != nullptr && RoomSubsystem)
	{
		ULevelStreamingDynamic* StreamingLevel = RoomSubsystem->SpawnRoomLevel(
			GeneratedMap[0].RoomDefinition,
			GeneratedMap[0].SpawnLocation,
			FRotator::ZeroRotator
		);

		if (StreamingLevel)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] %d층 0번 방 스폰 지시 완료!"), CurrentFloorIndex + 1);
		}
	}
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

void AR1MapGenerator::OnTransitionRoomLoaded()
{
	if (PendingNodeID == -1) return;

	UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();

	if (RoomSubsystem)
	{
		RoomSubsystem->MarkRoomAsLeft(GeneratedMap[CurrentActiveNodeID].RoomDefinition);
	}

	FVector NextLocation = GeneratedMap[PendingNodeID].SpawnLocation;

	for (TActorIterator<ADungeonManager> ManagerIt(GetWorld()); ManagerIt; ++ManagerIt)
	{
		FVector2D ManagerLoc2D(ManagerIt->GetActorLocation().X, ManagerIt->GetActorLocation().Y);
		FVector2D NextLoc2D(NextLocation.X, NextLocation.Y);

		if (FVector2D::Distance(ManagerLoc2D, NextLoc2D) < 100.0f)
		{
			RegisterRoomManager(*ManagerIt);
			break;
		}
	}

	TriggerAutoSave();
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

			UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 💾 자동 저장 완료! (현재 저장된 방: %d번)"), CurrentActiveNodeID);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[MapGenerator] ❌ 자동 저장 실패: 플레이어 캐릭터를 찾을 수 없습니다!"));
		}
	}
}
