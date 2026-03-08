

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "Map/R1MapGenerator.h"
#include "R1MinimapWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1MinimapWidget : public UR1UserWidget
{
	GENERATED_BODY()
protected:
	// 위젯이 화면에 생성될 때 자동으로 호출되는 초기화 함수
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> CanvasPanel_Entries;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	TSubclassOf<class UR1MinimapRoomWidget> RoomWidgetClass;

	UPROPERTY()
	TMap<int32, class UR1MinimapRoomWidget*> SpawnedRooms;

	// 방 1개의 UI 크기
	const float ROOM_SIZE = 50.0f;

public:
	// 맵 제너레이터의 델리게이트와 연결될 콜백 함수들
	UFUNCTION()
	void OnMapGeneratedCallback(const TArray<FR1MapNode>& MapData);

	UFUNCTION()
	void OnPlayerMovedRoomCallback(int32 NewRoomNodeID, int32 PrevRoomNodeID);

private:
	void UpdateMinimapUI(int32 CurrentRoomID, AR1MapGenerator* Generator);

private:
	UPROPERTY()
	FVector2D MapCenterOffset = FVector2D::ZeroVector;

	// 전체 맵 데이터(바운딩 박스)를 바탕으로 중심점을 계산하는 함수
	void CalculateMapCenterOffset(const TArray<FR1MapNode>& MapData);
};
