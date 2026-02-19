#include "Map/R1MapGenerator.h"
#include "Map/DungeonManager.h"
#include "Map/R1Door.h"

#include "Data/R1RoomDefinitionData.h"
#include "System/R1RoomStreamingSubsystem.h"

#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

#include "Containers/Queue.h"
#include "Data/R1AssetData.h" 
#include "EngineUtils.h"


AR1MapGenerator::AR1MapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AR1MapGenerator::BeginPlay()
{
	Super::BeginPlay();

	InitializeRoomPools();
	// 1. 게임이 시작되면 아이작 알고리즘으로 맵(배열 데이터)을 먼저 완성합니다.
	GenerateMap();

	// 2. 지도가 완성되었다면, 첫 번째 방(시작 방)을 실제로 스폰합니다.
	if (GeneratedMap.Num() > 0 && GeneratedMap[0].RoomDefinition != nullptr)
	{
		UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
		if (RoomSubsystem)
		{
			// 서브시스템에 0번 방 스폰을 요청
			ULevelStreamingDynamic* StreamingLevel = RoomSubsystem->SpawnRoomLevel(
				GeneratedMap[0].RoomDefinition,
				GeneratedMap[0].SpawnLocation,
				FRotator::ZeroRotator
			);

			// 스폰 지시가 성공적으로 들어갔다면, 완료 델리게이트 연결
			if (StreamingLevel)
			{
				UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 0번 방 스폰을 요청했습니다. 대기 중..."));
				StreamingLevel->OnLevelLoaded.AddDynamic(this, &AR1MapGenerator::OnRoomLoaded);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MapGenerator] 맵이 생성되지 않았거나 시작 방 데이터가 없습니다!"));
	}
}

void AR1MapGenerator::GenerateMap()
{
	int32 MaxRetries = 50; // 확률로 인해 맵이 덜 만들어졌을 때 재시도할 최대 횟수
	bool bMapGeneratedSuccessfully = false;

	while (MaxRetries > 0 && !bMapGeneratedSuccessfully)
	{
		GeneratedMap.Empty();
		TQueue<int32> RoomQueue;
		int32 CurrentNodeID = 0;

		// 1. 시작 방 생성 (그리드의 중심인 0,0 좌표에서 시작)
		FR1MapNode StartNode;
		StartNode.NodeID = CurrentNodeID;
		StartNode.GridPosition = FIntPoint(0, 0);
		StartNode.SpawnLocation = FVector(0.0f, 0.0f, 0.0f);

		GeneratedMap.Add(StartNode);
		RoomQueue.Enqueue(CurrentNodeID);
		CurrentNodeID++;

		// 2. BFS 방식 가지치기 루프
		while (!RoomQueue.IsEmpty() && CurrentNodeID < TotalRoomCount)
		{
			int32 ParentID;
			RoomQueue.Dequeue(ParentID);	//ParentID에 현재 방 번호가 들어감.

			// 원본 배열 참조가 깨질 수 있으므로 좌표값만 복사해서 사용
			FIntPoint ParentPos = GeneratedMap[ParentID].GridPosition;

			// 방향 무작위 셔플 (한쪽으로만 뻗어나가는 것을 방지)
			TArray<FIntPoint> Directions = { FIntPoint(0, 1), FIntPoint(0, -1), FIntPoint(-1, 0), FIntPoint(1, 0) };
			for (int32 i = Directions.Num() - 1; i > 0; i--)
			{
				int32 j = FMath::RandRange(0, i);
				Directions.Swap(i, j);
			}

			// 인접한 4방향 탐색
			for (FIntPoint Dir : Directions)
			{
				if (CurrentNodeID >= TotalRoomCount) break; // 목표 개수 달성 시 중단

				FIntPoint NewPos = ParentPos + Dir;

				// 이미 이 자리에 방이 있는지 체크
				if (HasRoomAt(NewPos)) continue;

				// 뭉침 방지: 새 위치 주변에 방이 2개 이상 있다면 생성 취소
				int32 NeighborCount = 0;
				FIntPoint CheckDirs[4] = { FIntPoint(0, 1), FIntPoint(0, -1), FIntPoint(-1, 0), FIntPoint(1, 0) };
				for (FIntPoint CheckDir : CheckDirs)
				{
					if (HasRoomAt(NewPos + CheckDir)) NeighborCount++;
				}

				if (NeighborCount > 1) continue;

				// 50% 확률로 새 방 생성
				if (FMath::RandRange(0, 100) < 50)
				{
					FR1MapNode NewNode;
					NewNode.NodeID = CurrentNodeID;
					NewNode.GridPosition = NewPos;

					// 실제 월드 좌표 = 그리드 좌표 * 우리가 정한 방 간격
					NewNode.SpawnLocation = FVector(NewPos.X * RoomSpacing, NewPos.Y * RoomSpacing, 0.0f);

					// 서로 연결 상태 기록
					NewNode.ConnectedNodeIDs.Add(ParentID);
					GeneratedMap.Add(NewNode);
					GeneratedMap[ParentID].ConnectedNodeIDs.Add(CurrentNodeID);

					RoomQueue.Enqueue(CurrentNodeID);
					CurrentNodeID++;
				}
			}
		}

		// 목표한 방 개수를 정확히 채웠는지 확인
		if (GeneratedMap.Num() == TotalRoomCount)
		{
			bMapGeneratedSuccessfully = true;
		}

		MaxRetries--;
	}

	// 3. 지도가 완성되었다면, 데이터(PDA)를 각 방에 할당합니다.
	if (bMapGeneratedSuccessfully)
	{
		AssignRoomTypes();
		UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 총 %d개의 방 지도 생성이 완료되었습니다!"), GeneratedMap.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MapGenerator] 맵 생성 실패: 확률 문제로 목표 개수 미달. 다시 플레이해주세요."));
	}
}

void AR1MapGenerator::InitializeRoomPools()
{
	if (!GlobalAssetData)
	{
		UE_LOG(LogTemp, Error, TEXT("[MapGenerator] GlobalAssetData가 할당되지 않았습니다!"));
		return;
	}

	// 람다(Lambda) 함수를 활용해 라벨 이름만 주면 알아서 풀을 채우도록 만듭니다.
	auto LoadPoolByLabel = [&](FName Label, TArray<UR1RoomDefinitionData*>& OutPool)
		{
			OutPool.Empty();

			// 1. 라벨에 해당하는 에셋 목록을 글로벌 데이터에서 긁어옵니다.
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

			UE_LOG(LogTemp, Log, TEXT("[MapGenerator] '%s' 풀 로드 완료: %d개"), *Label.ToString(), OutPool.Num());
		};

	// 이제 원하는 라벨만 던져주면 끝입니다!
	LoadPoolByLabel(FName("Room_Start"), StartRoomPool);
	LoadPoolByLabel(FName("Room_Combat"), CombatRoomPool);
	LoadPoolByLabel(FName("Room_Boss"), BossRoomPool);
}

void AR1MapGenerator::AssignRoomTypes()
{
	// 에디터에서 풀을 안 채워놨을 경우를 대비한 방어 로직
	if (StartRoomPool.IsEmpty() || CombatRoomPool.IsEmpty() || BossRoomPool.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[MapGenerator] Room Pool이 비어있습니다. 에디터에서 PDA를 채워주세요!"));
		return;
	}

	// 0번 방(시작 지점)은 무조건 시작 방 세팅
	GeneratedMap[0].RoomDefinition = StartRoomPool[FMath::RandRange(0, StartRoomPool.Num() - 1)];
	GeneratedMap[0].bIsCleared = true;

	int32 FurthestNodeID = -1;
	float MaxDistance = -1.0f;

	// 1번 방부터 순회하며 속성 찾기
	for (int32 i = 1; i < GeneratedMap.Num(); ++i)
	{
		// 기본적으로 모든 방을 일반(Combat) 방으로 깔아둡니다.
		GeneratedMap[i].RoomDefinition = CombatRoomPool[FMath::RandRange(0, CombatRoomPool.Num() - 1)];

		// 연결된 방이 1개뿐이라면 = '막다른 길(Dead End)'
		if (GeneratedMap[i].ConnectedNodeIDs.Num() == 1)
		{
			// 시작점(0,0)으로부터의 거리를 계산합니다.
			float Dist = FVector::Dist(FVector::ZeroVector, FVector(GeneratedMap[i].GridPosition.X, GeneratedMap[i].GridPosition.Y, 0));
			if (Dist > MaxDistance)
			{
				MaxDistance = Dist;
				FurthestNodeID = i;
			}
		}
	}

	// 시작 방에서 가장 멀리 떨어진 '막다른 길'을 찾아 '보스 방'으로 덮어씌웁니다.
	if (FurthestNodeID != -1)
	{
		GeneratedMap[FurthestNodeID].RoomDefinition = BossRoomPool[FMath::RandRange(0, BossRoomPool.Num() - 1)];
		UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 보스 방이 %d번 노드에 할당되었습니다."), FurthestNodeID);
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
	case ER1DoorDirection::North: TargetGridPos.Y += 1; break;
	case ER1DoorDirection::South: TargetGridPos.Y -= 1; break;
	case ER1DoorDirection::East:  TargetGridPos.X += 1; break;
	case ER1DoorDirection::West:  TargetGridPos.X -= 1; break;
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
		// 이미 텔레포트 진행 중이면 중복 실행 방지
		if (PendingNodeID != -1) return;

		UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 문 통과 감지! %d번 방에서 %d번 방으로 텔레포트를 시도합니다."), CurrentActiveNodeID, NextNodeID);

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
					UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 방이 이미 로드되어 있습니다. 즉시 텔레포트 진행!"));
					OnTransitionRoomLoaded();
				}
				// 아니라면 로딩 완료 시점에 호출되도록 델리게이트 연결
				else
				{
					UE_LOG(LogTemp, Log, TEXT("[MapGenerator] 방 비동기 로딩 중... 완료되면 텔레포트합니다."));
					StreamingLevel->OnLevelLoaded.AddUniqueDynamic(this, &AR1MapGenerator::OnTransitionRoomLoaded);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[MapGenerator] 방 스폰에 실패했습니다!"));
				PendingNodeID = -1;
			}
		}
	}
}

void AR1MapGenerator::OnRoomLoaded()
{
	UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 0번 시작 방 로딩 완료! 플레이어를 안전하게 이동시킵니다."));

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (PlayerCharacter)
	{
		FVector SafeLocation = GeneratedMap[0].SpawnLocation + FVector(0.0f, 0.0f, 150.0f);
		PlayerCharacter->SetActorLocation(SafeLocation);
		PlayerCharacter->GetVelocity() = FVector::ZeroVector;
	}

	UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
	if (RoomSubsystem)
	{
		RoomSubsystem->MarkRoomGameplayReady(GeneratedMap[0].RoomDefinition);
	}

	FTimerHandle SetupTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(SetupTimerHandle, FTimerDelegate::CreateLambda([this]()
		{
			FVector CurrentRoomLocation = GeneratedMap[CurrentActiveNodeID].SpawnLocation;

			ADungeonManager* CurrentRoomManager = nullptr;
			for (TActorIterator<ADungeonManager> ManagerIt(GetWorld()); ManagerIt; ++ManagerIt)
			{
				if (ManagerIt->GetActorLocation().Equals(CurrentRoomLocation, 10.0f))
				{
					CurrentRoomManager = *ManagerIt;
					break;
				}
			}

			// 2. 매니저가 스포이드로 이미 들고 있는 문들만 세팅합니다. (문을 찾는 for문 삭제!)
			if (CurrentRoomManager)
			{
				for (AR1Door* Door : CurrentRoomManager->RoomDoors)
				{
					if (!IsValid(Door)) continue;

					int32 TargetNode = GetConnectedNodeInDirection(CurrentActiveNodeID, Door->DoorDirection);
					Door->SetupDoorConnection(TargetNode);

					if (TargetNode != -1)
					{
						Door->OnDoorEntered.RemoveDynamic(this, &AR1MapGenerator::OnPlayerEnteredDoor);
						Door->OnDoorEntered.AddDynamic(this, &AR1MapGenerator::OnPlayerEnteredDoor);
					}
				}

				CurrentRoomManager->RoomNodeID = CurrentActiveNodeID;
				CurrentRoomManager->OnRoomCleared.RemoveDynamic(this, &AR1MapGenerator::OnRoomClearedCallback);
				CurrentRoomManager->OnRoomCleared.AddDynamic(this, &AR1MapGenerator::OnRoomClearedCallback);

				if (GeneratedMap[CurrentActiveNodeID].bIsCleared)
				{
					CurrentRoomManager->bIsCleared = true;
					CurrentRoomManager->UnlockRoomDoors();
				}
				CurrentRoomManager->StartRoomCombat();
			}

		}), 0.1f, false);
}

void AR1MapGenerator::OnTransitionRoomLoaded()
{
	if (PendingNodeID == -1) return;

	UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();

	if (RoomSubsystem)
	{
		RoomSubsystem->MarkRoomAsLeft(GeneratedMap[CurrentActiveNodeID].RoomDefinition);
	}

	// 1. [수정] 자물쇠를 여기서 풀지 않고, 목적지 번호만 임시 저장합니다.
	int32 TargetRoomID = PendingNodeID;

	FVector NewRoomLocation = GeneratedMap[TargetRoomID].SpawnLocation;
	ER1DoorDirection OppositeDir = GetOppositeDirection(PendingDoorDirection);

	FTimerHandle TransitionTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TransitionTimerHandle, FTimerDelegate::CreateLambda([this, TargetRoomID, NewRoomLocation, OppositeDir]()
		{
			CurrentActiveNodeID = TargetRoomID;
			AR1Door* TargetDoorToSpawnAt = nullptr;
			ADungeonManager* CurrentRoomManager = nullptr;

			// 1. [핀포인트 매칭]
			for (TActorIterator<ADungeonManager> ManagerIt(GetWorld()); ManagerIt; ++ManagerIt)
			{
				if (ManagerIt->GetActorLocation().Equals(NewRoomLocation, 10.0f))
				{
					CurrentRoomManager = *ManagerIt;
					break;
				}
			}

			// 2. 지휘관이 들고 있는 문만 세팅
			if (CurrentRoomManager)
			{
				for (AR1Door* Door : CurrentRoomManager->RoomDoors)
				{
					if (!IsValid(Door)) continue;

					int32 TargetNode = GetConnectedNodeInDirection(CurrentActiveNodeID, Door->DoorDirection);
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

				CurrentRoomManager->RoomNodeID = CurrentActiveNodeID;
				CurrentRoomManager->OnRoomCleared.RemoveDynamic(this, &AR1MapGenerator::OnRoomClearedCallback);
				CurrentRoomManager->OnRoomCleared.AddDynamic(this, &AR1MapGenerator::OnRoomClearedCallback);

				if (GeneratedMap[CurrentActiveNodeID].bIsCleared)
				{
					CurrentRoomManager->bIsCleared = true;
					CurrentRoomManager->UnlockRoomDoors();
				}
				else
				{
					CurrentRoomManager->LockRoomDoors();
				}
				CurrentRoomManager->StartRoomCombat();
			}

			// 3. 플레이어 텔레포트
			ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
			if (PlayerCharacter)
			{
				if (TargetDoorToSpawnAt)
				{
					FVector DirectionToCenter = (NewRoomLocation - TargetDoorToSpawnAt->GetActorLocation()).GetSafeNormal();
					FVector SafeLocation = TargetDoorToSpawnAt->GetActorLocation() + (DirectionToCenter * 300.0f) + FVector(0.0f, 0.0f, 100.0f);
					PlayerCharacter->SetActorLocation(SafeLocation);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("[MapGenerator] 타겟 문 없음! 방 중앙으로 비상 이동!"));
					FVector EmergencyLocation = NewRoomLocation + FVector(0.0f, 0.0f, 150.0f);
					PlayerCharacter->SetActorLocation(EmergencyLocation);
				}
				PlayerCharacter->GetVelocity() = FVector::ZeroVector;
			}

			PendingNodeID = -1;

			// 8. 주변 방 선로딩
			UR1RoomStreamingSubsystem* InnerRoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
			if (InnerRoomSubsystem)
			{
				InnerRoomSubsystem->MarkRoomGameplayReady(GeneratedMap[CurrentActiveNodeID].RoomDefinition);

				TArray<UR1RoomDefinitionData*> AdjacentRooms;
				for (int32 ConnectedID : GeneratedMap[CurrentActiveNodeID].ConnectedNodeIDs)
				{
					AdjacentRooms.Add(GeneratedMap[ConnectedID].RoomDefinition);
				}
				InnerRoomSubsystem->QueuePreloadRooms(AdjacentRooms);
			}

		}), 0.1f, false);
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

		UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 수신 완료! %d번 방의 지도 데이터가 클리어 상태로 영구 저장되었습니다!"), ClearedNodeID);
	}
}
