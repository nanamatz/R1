

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1ConfirmModalSceneWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1ConfirmModalSceneWidget : public UR1UserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1ConfirmModal> WBP_ConfirmModal;
};
