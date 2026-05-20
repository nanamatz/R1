

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "UI/System/R1GameOptionsMenuWidget.h"
#include "R1GameOptionsMenuSceneWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1GameOptionsMenuSceneWidget : public UR1UserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UR1GameOptionsMenuWidget> GameOptionsMenuWidget;
};
