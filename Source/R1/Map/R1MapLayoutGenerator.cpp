#include "Map/R1MapLayoutGenerator.h"
#include "Map/R1MapGrid.h"
#include "Data/R1RoomDefinitionData.h"
#include "Containers/Queue.h"

bool FR1MapLayoutGenerator::BuildAttempt(TArray<UR1RoomDefinitionData*>& StartPool,
	TArray<UR1RoomDefinitionData*>& CombatPool,
	TArray<FR1MapNode>& OutMap) const
{
	OutMap.Empty();

	TQueue<int32> RoomQueue;
	int32 CurrentNodeID = 0;

	// 1. 시작 방 배정 (풀에서 1개 빼오기)
	FR1MapNode StartNode;
	StartNode.NodeID = CurrentNodeID;
	StartNode.GridPosition = FIntPoint(0, 0);
	StartNode.SpawnLocation = FVector::ZeroVector;

	if (StartPool.Num() > 0)
	{
		int32 StartIndex = FMath::RandRange(0, StartPool.Num() - 1);
		StartNode.RoomDefinition = StartPool[StartIndex];
		StartPool.RemoveAt(StartIndex);
	}
	StartNode.bIsCleared = true;

	OutMap.Add(StartNode);
	RoomQueue.Enqueue(CurrentNodeID);
	CurrentNodeID++;

	// 2. [핵심] 큐 순회 (퍼즐 맞추기)
	while (!RoomQueue.IsEmpty() && CurrentNodeID < TotalRoomCount)
	{
		int32 ParentID;
		RoomQueue.Dequeue(ParentID);

		// 아래 루프에서 OutMap.Add가 배열을 재할당하면 OutMap 원소 참조가 무효화된다.
		// 부모의 불변 정보(룸 데이터·그리드 좌표)는 미리 값으로 복사해 두고, 쓰기는 항상
		// OutMap[ParentID]로 재인덱싱한다. (이전엔 참조를 루프 내내 들고 있어 dangling 소지)
		const UR1RoomDefinitionData* ParentRoomDef = OutMap[ParentID].RoomDefinition;
		if (!ParentRoomDef) continue;

		const FIntPoint ParentGridPosition = OutMap[ParentID].GridPosition;

		// 현재 방에 실제로 뚫려있는 문 방향만 가져옵니다! (무지성 동서남북 배제)
		TArray<ER1DoorDirection> ParentDoors = ParentRoomDef->AvailableDoors;

		// 가지가 한쪽으로만 뻗는 걸 막기 위해 문 방향 셔플
		for (int32 i = ParentDoors.Num() - 1; i > 0; i--)
		{
			ParentDoors.Swap(i, FMath::RandRange(0, i));
		}

		// 실제 뚫려있는 문을 향해서만 가지를 뻗습니다.
		for (ER1DoorDirection DoorDir : ParentDoors)
		{
			if (CurrentNodeID >= TotalRoomCount) break;

			const FIntPoint DirOffset = R1MapGrid::GetGridOffset(DoorDir);
			const ER1DoorDirection OppositeDir = R1MapGrid::GetOppositeDirection(DoorDir);

			FIntPoint NewPos = ParentGridPosition + DirOffset;

			// 이미 그 위치에 다른 방이 있다면, 연결만 해주고 스킵
			if (int32 ExistingID = GetNodeIDAt(OutMap, NewPos); ExistingID != -1)
			{
				OutMap[ParentID].ConnectedNodeIDs.AddUnique(ExistingID);
				OutMap[ExistingID].ConnectedNodeIDs.AddUnique(ParentID);
				continue;
			}

			// 뭉침 방지 룰 (아이작 룰)
			int32 NeighborCount = 0;
			FIntPoint CheckDirs[4] = { FIntPoint(0, 1), FIntPoint(0, -1), FIntPoint(-1, 0), FIntPoint(1, 0) };
			for (FIntPoint CheckDir : CheckDirs)
			{
				if (GetNodeIDAt(OutMap, NewPos + CheckDir) != -1) NeighborCount++;
			}
			if (NeighborCount > 1) continue;

			// 확률 50%
			if (FMath::RandRange(0, 100) < 50)
			{
				// [가장 중요] 다음 방은 반드시 내 문과 맞물리는(OppositeDir) 문이 있어야 합니다!
				UR1RoomDefinitionData* NextRoomData = PopValidRoomFromPool(CombatPool, OppositeDir);

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
				OutMap.Add(NewNode);
				OutMap[ParentID].ConnectedNodeIDs.Add(CurrentNodeID);

				RoomQueue.Enqueue(CurrentNodeID);
				CurrentNodeID++;
			}
		}
	}

	return OutMap.Num() == TotalRoomCount;
}

int32 FR1MapLayoutGenerator::GetNodeIDAt(const TArray<FR1MapNode>& Map, FIntPoint Pos)
{
	for (int32 i = 0; i < Map.Num(); ++i)
	{
		if (Map[i].GridPosition == Pos) return Map[i].NodeID;
	}
	return -1;
}

UR1RoomDefinitionData* FR1MapLayoutGenerator::PopValidRoomFromPool(TArray<UR1RoomDefinitionData*>& Pool, ER1DoorDirection RequiredDoor)
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
