


#include "R1MinimapWidget.h"
#include "R1MinimapRoomWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Map/R1MapGenerator.h"
#include "Data/R1RoomDefinitionData.h"

void UR1MinimapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!CanvasPanel_Entries || !RoomWidgetClass) return;

	AR1MapGenerator* MapGenerator = Cast<AR1MapGenerator>(UGameplayStatics::GetActorOfClass(this, AR1MapGenerator::StaticClass()));

	if (!MapGenerator) return;
	
	MapGenerator->OnMapGenerated.AddDynamic(this, &UR1MinimapWidget::OnMapGeneratedCallback);
	MapGenerator->OnPlayerMovedRoom.AddDynamic(this, &UR1MinimapWidget::OnPlayerMovedRoomCallback);
	
	if (MapGenerator->GeneratedMap.Num() > 0)
	{
		OnMapGeneratedCallback(MapGenerator->GeneratedMap);
	}
}


void UR1MinimapWidget::OnMapGeneratedCallback(const TArray<FR1MapNode>& MapData)
{
	if (!CanvasPanel_Entries || !RoomWidgetClass) return;

	if (MapData.IsEmpty()) return;

	if (AActor* GeneratorActor = UGameplayStatics::GetActorOfClass(this, AR1MapGenerator::StaticClass()))
	{
		UpdateMinimapUI(0, Cast<AR1MapGenerator>(GeneratorActor));
	}
}

void UR1MinimapWidget::OnPlayerMovedRoomCallback(int32 NewRoomNodeID, int32 PrevRoomNodeID)
{
	if (AActor* GeneratorActor = UGameplayStatics::GetActorOfClass(this, AR1MapGenerator::StaticClass()))
	{
		// 🌟 이동할 때는 이전 방 번호를 넘겨줍니다.
		UpdateMinimapUI(NewRoomNodeID, Cast<AR1MapGenerator>(GeneratorActor));
	}
}

void UR1MinimapWidget::UpdateMinimapUI(int32 CurrentRoomID, AR1MapGenerator* Generator)
{
	if (!CanvasPanel_Entries || !RoomWidgetClass || !Generator) return;
	if (!Generator->GeneratedMap.IsValidIndex(CurrentRoomID)) return;

	auto TrySpawnRoom = [&](int32 NodeID)
		{
			if (SpawnedRooms.Contains(NodeID)) return; // 이미 있으면 스킵

			const FR1MapNode& Node = Generator->GeneratedMap[NodeID];
			UR1MinimapRoomWidget* NewRoom = CreateWidget<UR1MinimapRoomWidget>(this, RoomWidgetClass);

			if (NewRoom)
			{
				UCanvasPanelSlot* CanvasSlot = CanvasPanel_Entries->AddChildToCanvas(NewRoom);
				if (CanvasSlot)
				{
					CanvasSlot->SetAnchors(FAnchors(0.5f));
					CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
					CanvasSlot->SetAutoSize(false);
					CanvasSlot->SetSize(FVector2D(ROOM_SIZE, ROOM_SIZE));

					FVector2D UIPosition(Node.GridPosition.Y * ROOM_SIZE, Node.GridPosition.X * -ROOM_SIZE);
					CanvasSlot->SetPosition(UIPosition);
				}

				SpawnedRooms.Add(NodeID, NewRoom);
			}
		};

	// 제너레이터가 방금 상태를 다 갱신해 줬으니, 우리는 현재 방과 연결된 방만 화면에 생성하면 끝입니다!
	TrySpawnRoom(CurrentRoomID);

	const FR1MapNode& CurrentNode = Generator->GeneratedMap[CurrentRoomID];
	for (int32 ConnectedID : CurrentNode.ConnectedNodeIDs)
	{
		TrySpawnRoom(ConnectedID);
	}

	// ==========================================
	// ✨ 2단계: 화면에 띄워진 모든 UI의 색상 최신화
	// ==========================================
	for (auto& Pair : SpawnedRooms)
	{
		int32 NodeID = Pair.Key;
		UR1MinimapRoomWidget* RoomWidget = Pair.Value;

		// 제너레이터가 이미 UpdateMinimapState()로 바꿔둔 최신 데이터를 그대로 가져와서 꽂기만 합니다.
		const FR1MapNode& Node = Generator->GeneratedMap[NodeID];
		ER1RoomContentType RoomType = Node.RoomDefinition ? Node.RoomDefinition->RoomType : ER1RoomContentType::Combat;

		RoomWidget->UpdateRoomState(Node.MinimapState, RoomType);
	}
}

//void UR1MinimapWidget::UpdateMinimapUI(int32 CurrentRoomID, int32 PrevRoomID, AR1MapGenerator* Generator)
//{
	//if (!CanvasPanel_Entries || !RoomWidgetClass || !Generator) return;
	//if (!Generator->GeneratedMap.IsValidIndex(CurrentRoomID)) return;

	//// ==========================================
	//// 🧠 1단계: 데이터 상태 업데이트 (State Transition)
	//// ==========================================

	//// ① 플레이어가 방금 떠난 '이전 방'은 [Visited]로 변경합니다.
	//if (Generator->GeneratedMap.IsValidIndex(PrevRoomID))
	//{
	//	// 혹시라도 다른 상태를 덮어쓰지 않도록, Current였을 때만 Visited로 강등시킵니다.
	//	if (Generator->GeneratedMap[PrevRoomID].MinimapState == ER1MinimapRoomState::Current)
	//	{
	//		Generator->GeneratedMap[PrevRoomID].MinimapState = ER1MinimapRoomState::Visited;
	//	}
	//}

	//// ② 플레이어가 진입한 '현재 방'은 [Current]로 변경합니다.
	//Generator->GeneratedMap[CurrentRoomID].MinimapState = ER1MinimapRoomState::Current;

	//// ③ 현재 방의 4방향을 탐색하여 '인접한 방'들을 찾고 상태를 갱신합니다.
	//TArray<ER1DoorDirection> Directions = { ER1DoorDirection::North, ER1DoorDirection::South, ER1DoorDirection::East, ER1DoorDirection::West };
	//TArray<int32> AdjacentRoomIDs;

	//for (ER1DoorDirection Dir : Directions)
	//{
	//	int32 ConnectedID = Generator->GetConnectedNodeInDirection(CurrentRoomID, Dir);
	//	if (ConnectedID != -1)
	//	{
	//		AdjacentRoomIDs.Add(ConnectedID); // 인접 방 목록에 추가

	//		// 인접한 방이 아직 숨겨져(Hidden) 있다면, 드디어 발견(Discovered)된 것입니다!
	//		// (이미 Visited거나 Current인 방은 건드리지 않습니다)
	//		if (Generator->GeneratedMap[ConnectedID].MinimapState == ER1MinimapRoomState::Hidden)
	//		{
	//			Generator->GeneratedMap[ConnectedID].MinimapState = ER1MinimapRoomState::Discovered;
	//		}
	//	}
	//}

	//// ==========================================
	//// 🎨 2단계: UI 위젯 생성 및 배치
	//// ==========================================

	//auto TrySpawnRoom = [&](int32 NodeID)
	//	{
	//		if (SpawnedRooms.Contains(NodeID)) return;

	//		const FR1MapNode& Node = Generator->GeneratedMap[NodeID];
	//		UR1MinimapRoomWidget* NewRoom = CreateWidget<UR1MinimapRoomWidget>(this, RoomWidgetClass);

	//		if (NewRoom)
	//		{
	//			UCanvasPanelSlot* CanvasSlot = CanvasPanel_Entries->AddChildToCanvas(NewRoom);
	//			if (CanvasSlot)
	//			{
	//				CanvasSlot->SetAnchors(FAnchors(0.5f));
	//				CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	//				CanvasSlot->SetAutoSize(false);
	//				CanvasSlot->SetSize(FVector2D(ROOM_SIZE, ROOM_SIZE));

	//				FVector2D UIPosition(Node.GridPosition.Y * ROOM_SIZE, Node.GridPosition.X * -ROOM_SIZE);
	//				CanvasSlot->SetPosition(UIPosition);
	//			}

	//			SpawnedRooms.Add(NodeID, NewRoom);
	//		}
	//	};

	//// 현재 방의 UI를 그립니다.
	//TrySpawnRoom(CurrentRoomID);

	//// 인접한 방(Discovered)들의 UI도 미리 그려둡니다.
	//for (int32 AdjID : AdjacentRoomIDs)
	//{
	//	TrySpawnRoom(AdjID);
	//}

	//// ==========================================
	//// ✨ 3단계: 화면에 띄워진 모든 UI의 색상/상태 최신화
	//// ==========================================

	//// TMap에 저장된 모든 위젯을 순회하며, 최신화된 GeneratedMap의 상태를 주입합니다.
	//for (auto& Pair : SpawnedRooms)
	//{
	//	int32 NodeID = Pair.Key;
	//	UR1MinimapRoomWidget* RoomWidget = Pair.Value;

	//	// 1단계에서 갱신한 최신 상태를 가져옵니다.
	//	const FR1MapNode& Node = Generator->GeneratedMap[NodeID];
	//	ER1RoomContentType RoomType = Node.RoomDefinition ? Node.RoomDefinition->RoomType : ER1RoomContentType::Combat;

	//	// 🌟 여기서 WBP_MinimapRoom의 텍스처(Current, Visited, Discovered)가 교체됩니다!
	//	RoomWidget->UpdateRoomState(Node.MinimapState, RoomType);
	//}
//}
