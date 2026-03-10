


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

	if (CanvasPanel_Entries)
	{
		CanvasPanel_Entries->ClearChildren();
	}

	SpawnedRooms.Empty();

	CalculateMapCenterOffset(MapData);

	if (AR1MapGenerator* MapGenerator = Cast<AR1MapGenerator>(UGameplayStatics::GetActorOfClass(this, AR1MapGenerator::StaticClass())))
	{
		// 0번이 아니라, 현재 제너레이터가 들고 있는 진짜 현재 방 번호를 넘겨야 합니다 (세이브 로드 대응)
		UpdateMinimapUI(MapGenerator->CurrentActiveNodeID, MapGenerator);
	}
}

void UR1MinimapWidget::OnPlayerMovedRoomCallback(int32 NewRoomNodeID, int32 PrevRoomNodeID)
{
	if (AR1MapGenerator* MapGenerator = Cast<AR1MapGenerator>(UGameplayStatics::GetActorOfClass(this, AR1MapGenerator::StaticClass())))
	{
		UpdateMinimapUI(NewRoomNodeID, MapGenerator);
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

					float OffsetGridX = Node.GridPosition.X - MapCenterOffset.X;
					float OffsetGridY = Node.GridPosition.Y - MapCenterOffset.Y;

					FVector2D UIPosition(OffsetGridY * ROOM_SIZE, OffsetGridX * -ROOM_SIZE);
					CanvasSlot->SetPosition(UIPosition);
				}

				SpawnedRooms.Add(NodeID, NewRoom);
			}
		};

	// [수정 포인트] 세이브 로드 시 이미 방문했거나 발견된 방들이 많을 수 있습니다.
	// 단순히 현재 방의 인접 방만 만드는 게 아니라, Hidden이 아닌 모든 방을 검사해서 생성해야 합니다.
	for (const FR1MapNode& Node : Generator->GeneratedMap)
	{
		if (Node.MinimapState != ER1MinimapRoomState::Hidden)
		{
			TrySpawnRoom(Node.NodeID);
		}
	}

	for (auto& Pair : SpawnedRooms)
	{
		int32 NodeID = Pair.Key;
		UR1MinimapRoomWidget* RoomWidget = Pair.Value;

		const FR1MapNode& Node = Generator->GeneratedMap[NodeID];
		ER1RoomContentType RoomType = Node.RoomDefinition ? Node.RoomDefinition->RoomType : ER1RoomContentType::Combat;

		RoomWidget->UpdateRoomState(Node.MinimapState, RoomType);
	}
}

void UR1MinimapWidget::CalculateMapCenterOffset(const TArray<FR1MapNode>& MapData)
{
	if (MapData.IsEmpty()) return;

	// 초기값을 0번 방의 좌표로 설정
	int32 MinX = MapData[0].GridPosition.X;
	int32 MaxX = MapData[0].GridPosition.X;
	int32 MinY = MapData[0].GridPosition.Y;
	int32 MaxY = MapData[0].GridPosition.Y;

	// 모든 방을 순회하며 가장 끝에 있는 좌표들을 찾음
	for (const FR1MapNode& Node : MapData)
	{
		MinX = FMath::Min(MinX, Node.GridPosition.X);
		MaxX = FMath::Max(MaxX, Node.GridPosition.X);
		MinY = FMath::Min(MinY, Node.GridPosition.Y);
		MaxY = FMath::Max(MaxY, Node.GridPosition.Y);
	}

	// 맵 전체 모양의 '정중앙(Center)' 그리드 좌표 계산
	MapCenterOffset.X = (MinX + MaxX) / 2.0f;
	MapCenterOffset.Y = (MinY + MaxY) / 2.0f;

	UE_LOG(LogTemp, Warning, TEXT("[Minimap] 맵 오프셋 계산 완료: X=%f, Y=%f"), MapCenterOffset.X, MapCenterOffset.Y);
}
