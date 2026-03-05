


#include "R1MinimapWidget.h"
#include "R1MinimapRoomWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Data/R1RoomDefinitionData.h"

void UR1MinimapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AActor* GeneratorActor = UGameplayStatics::GetActorOfClass(this, AR1MapGenerator::StaticClass()))
	{
		if (AR1MapGenerator* Generator = Cast<AR1MapGenerator>(GeneratorActor))
		{
			// 확성기에 함수 연결!
			Generator->OnMapGenerated.AddDynamic(this, &UR1MinimapWidget::OnMapGeneratedCallback);
			Generator->OnPlayerMovedRoom.AddDynamic(this, &UR1MinimapWidget::OnPlayerMovedRoomCallback);
		}
	}

}

void UR1MinimapWidget::NativeDestruct()
{
	Super::NativeDestruct();
	// 이 빨간 로그가 찍힌다면, 로직 문제가 아니라 위젯 자체가 강제로 암살당한 것입니다!
	UE_LOG(LogTemp, Error, TEXT("[MinimapUI] 🚨 앗! 미니맵 위젯이 화면에서 삭제(파괴)되었습니다!! (NativeDestruct)"));
}

void UR1MinimapWidget::OnMapGeneratedCallback(const TArray<FR1MapNode>& MapData)
{
	//if (!CanvasPanel_Map || !RoomWidgetClass) return;

	//// 기존 데이터 초기화
	////CanvasPanel_Map->ClearChildren();
	////SpawnedRoomWidgets.Empty();

	//// 2. 전달받은 지도 데이터를 바탕으로 방 위젯들을 동적 생성합니다.
	//for (const FR1MapNode& Node : MapData)
	//{
	//	UR1MinimapRoomWidget* RoomWidget = CreateWidget<UR1MinimapRoomWidget>(this, RoomWidgetClass);
	//	if (RoomWidget)
	//	{
	//		// 캔버스에 추가
	//		UCanvasPanelSlot* CanvasSlot = CanvasPanel_Map->AddChildToCanvas(RoomWidget);
	//		if (CanvasSlot)
	//		{
	//			// 🌟 2D 그리드 좌표를 UI 픽셀 좌표로 변환 (UI는 아래로 갈수록 Y가 증가하므로 Y에 -1을 곱해 북쪽을 위로 맞춥니다)
	//			FVector2D UIPosition(Node.GridPosition.X * RoomSize, Node.GridPosition.Y * -RoomSize);

	//			CanvasSlot->SetPosition(UIPosition);
	//			CanvasSlot->SetSize(FVector2D(RoomSize, RoomSize));
	//			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f)); // 중앙 정렬

	//			UE_LOG(LogTemp, Log, TEXT("[MinimapUI] 방 위젯 생성 완료 - NodeID: %d, UI 좌표: X=%f, Y=%f"), Node.NodeID, UIPosition.X, UIPosition.Y);
	//		}

	//		// 방 초기 상태 설정
	//		ER1RoomContentType RoomType = Node.RoomDefinition ? Node.RoomDefinition->RoomType : ER1RoomContentType::Combat;
	//		RoomWidget->UpdateRoomState(Node.MinimapState, RoomType);

	//		// 나중에 업데이트하기 쉽게 장부에 기록
	//		SpawnedRoomWidgets.Add(Node.NodeID, RoomWidget);
	//	}
	//}
	if (!CanvasPanel_Map || !RoomWidgetClass) return;

	//CanvasPanel_Map->ClearChildren();
	//SpawnedRoomWidgets.Empty();

	for (const FR1MapNode& Node : MapData)
	{
		UR1MinimapRoomWidget* RoomWidget = CreateWidget<UR1MinimapRoomWidget>(this, RoomWidgetClass);
		if (RoomWidget)
		{
			UCanvasPanelSlot* CanvasSlot = CanvasPanel_Map->AddChildToCanvas(RoomWidget);
			if (CanvasSlot)
			{
				// 🌟 [핵심 해결책] C++에서도 앵커를 정중앙(Center)으로 강제 고정합니다!
				CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));

				// 🌟 이제 (0, 0) 좌표는 도화지의 정확히 한가운데가 됩니다.
				FVector2D UIPosition(Node.GridPosition.X * RoomSize, Node.GridPosition.Y * -RoomSize);

				CanvasSlot->SetPosition(UIPosition);
				CanvasSlot->SetAutoSize(false); // 크기를 수동으로 지정하겠다고 선언
				CanvasSlot->SetSize(FVector2D(RoomSize, RoomSize));
			}

			// 방 상태 업데이트
			ER1RoomContentType RoomType = Node.RoomDefinition ? Node.RoomDefinition->RoomType : ER1RoomContentType::Combat;
			RoomWidget->UpdateRoomState(Node.MinimapState, RoomType);

			SpawnedRoomWidgets.Add(Node.NodeID, RoomWidget);
		}
	}
}

void UR1MinimapWidget::OnPlayerMovedRoomCallback(int32 NewRoomNodeID, int32 PrevRoomNodeID)
{
	UE_LOG(LogTemp, Warning, TEXT("[MinimapUI] 플레이어 이동 이벤트 발생! 이전 방: %d -> 새 방: %d"), PrevRoomNodeID, NewRoomNodeID);
	// 3. 플레이어가 이동했다면, 맵 제너레이터의 최신 데이터를 가져와 모든 방의 상태를 새로고침합니다.
	if (AActor* GeneratorActor = UGameplayStatics::GetActorOfClass(this, AR1MapGenerator::StaticClass()))
	{
		if (AR1MapGenerator* Generator = Cast<AR1MapGenerator>(GeneratorActor))
		{
			for (const FR1MapNode& Node : Generator->GeneratedMap)
			{
				// 장부에서 해당 방 위젯을 찾아서 최신 상태 덮어씌우기
				if (UR1MinimapRoomWidget** FoundWidget = SpawnedRoomWidgets.Find(Node.NodeID))
				{
					ER1RoomContentType RoomType = Node.RoomDefinition ? Node.RoomDefinition->RoomType : ER1RoomContentType::Combat;
					(*FoundWidget)->UpdateRoomState(Node.MinimapState, RoomType);
				}
			}
		}
	}
}
