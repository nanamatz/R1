

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Map/R1MapGenerator.h"
#include "Data/R1RoomDefinitionData.h"
#include "R1MinimapRoomWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1MinimapRoomWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// 🌟 외부(전체 맵 위젯)에서 이 방의 상태를 갱신할 때 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void UpdateRoomState(ER1MinimapRoomState NewState, ER1RoomContentType RoomType);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_Background;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_Icon;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	TObjectPtr<UTexture2D> BgTexture_Current; 

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	TObjectPtr<UTexture2D> BgTexture_Visited;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	TObjectPtr<UTexture2D> BgTexture_Discovered;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	TMap<ER1RoomContentType, TObjectPtr<UTexture2D>> RoomIconMap;
};
