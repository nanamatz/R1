

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1Define.h"
#include "R1FloorGuideSceneWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1FloorGuideSceneWidget : public UR1UserWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1FloorGuideWidget> WBP_FloorGuide;

public:
	// HUD가 호출할 함수
	void ShowFloorGuide(ER1FloorLevel FloorLevel);

private:
	UFUNCTION()
	void HideScene();
};
