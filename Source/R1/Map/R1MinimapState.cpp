#include "Map/R1MinimapState.h"
#include "Map/R1MapGenerator.h" // FR1MapNode, ER1MinimapRoomState

void R1MinimapState::ApplyRoomEntered(TArray<FR1MapNode>& Nodes, int32 EnteredNodeID)
{
	// 1. 이전 'Current' 방을 강등.
	for (FR1MapNode& Node : Nodes)
	{
		if (Node.MinimapState == ER1MinimapRoomState::Current && Node.NodeID != EnteredNodeID)
		{
			// 클리어 여부에 따라 상태 결정 (만약 안 깬 방에서 도망쳐 나온 거라면 Discovered로 유지)
			Node.MinimapState = Node.bIsCleared ? ER1MinimapRoomState::Visited : ER1MinimapRoomState::Discovered;
		}
	}

	// 2. 새로 진입한 방을 'Current(현재 위치)'로 변경
	if (Nodes.IsValidIndex(EnteredNodeID))
	{
		Nodes[EnteredNodeID].MinimapState = ER1MinimapRoomState::Current;
		Nodes[EnteredNodeID].bIsVisited = true;

		// 3. 진입한 방과 연결된 모든 이웃 방들을 탐색하여 'Hidden'이면 'Discovered(발견됨)'로 밝힙니다.
		for (int32 ConnectedID : Nodes[EnteredNodeID].ConnectedNodeIDs)
		{
			if (Nodes.IsValidIndex(ConnectedID))
			{
				if (Nodes[ConnectedID].MinimapState == ER1MinimapRoomState::Hidden)
				{
					Nodes[ConnectedID].MinimapState = ER1MinimapRoomState::Discovered;
				}
			}
		}
	}
}
