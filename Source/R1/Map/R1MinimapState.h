#pragma once

#include "CoreMinimal.h"

struct FR1MapNode;

// 미니맵 방 탐색 상태(ER1MinimapRoomState) 전이 로직. 순수(부수효과는 전달된 노드 배열 변경뿐,
// 엔진/액터 의존 없음)라 단위 테스트가 가능하다. AR1MapGenerator God Class에서 분리(C-1.4).
// 상태 자체는 FR1MapNode에 있고 세이브에 직렬화되므로 소유권은 맵 데이터에 남는다(미니맵 위젯은 읽기 전용 렌더링).
namespace R1MinimapState
{
	// 플레이어가 EnteredNodeID 방에 진입했을 때의 미니맵 상태 갱신:
	// 이전 Current 방을 클리어 여부에 따라 Visited/Discovered로 강등하고,
	// 진입 방을 Current(+방문 처리)로, 그 이웃 중 Hidden인 방을 Discovered로 밝힌다.
	void ApplyRoomEntered(TArray<FR1MapNode>& Nodes, int32 EnteredNodeID);
}
