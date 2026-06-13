#pragma once

#include "CoreMinimal.h"
#include "Map/R1MapGenerator.h" // FR1MapNode, ER1DoorDirection 정의

class UR1RoomDefinitionData;

// AR1MapGenerator God Class에서 분리한 방 그래프 토폴로지 생성 알고리즘 (C-1.2).
// 액터 인스턴스(this)·UPROPERTY 멤버에 의존하지 않으므로 단위 테스트가 가능하다.
// RNG(FMath::RandRange) 호출 순서는 원본 GenerateMap의 '한 번 시도'와 정확히 동일하게 보존한다.
// (방 풀 로딩/재시도 루프/특수방 배정/오토세이브는 호출부 액터에 남는다.)
class FR1MapLayoutGenerator
{
public:
	FR1MapLayoutGenerator(int32 InTotalRoomCount, float InRoomSpacing)
		: TotalRoomCount(InTotalRoomCount)
		, RoomSpacing(InRoomSpacing)
	{
	}

	// 한 번의 BFS 시도로 OutMap에 방 그래프를 만든다(시작 시 OutMap을 비운다).
	// 배치에 사용한 방은 StartPool/CombatPool에서 제거(소비)한다.
	// 목표 방 개수(TotalRoomCount)에 도달하면 true. 실패 시 호출부가 풀을 다시 채워 재시도한다.
	bool BuildAttempt(TArray<UR1RoomDefinitionData*>& StartPool,
		TArray<UR1RoomDefinitionData*>& CombatPool,
		TArray<FR1MapNode>& OutMap) const;

private:
	// 좌표에 이미 방이 있으면 그 NodeID, 없으면 -1.
	static int32 GetNodeIDAt(const TArray<FR1MapNode>& Map, FIntPoint Pos);

	// 풀에서 RequiredDoor 문을 가진 방을 무작위 순서로 탐색해 제거 후 반환. 없으면 nullptr.
	static UR1RoomDefinitionData* PopValidRoomFromPool(TArray<UR1RoomDefinitionData*>& Pool, ER1DoorDirection RequiredDoor);

	int32 TotalRoomCount;
	float RoomSpacing;
};
