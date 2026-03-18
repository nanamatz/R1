

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1InventoryMainWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1InventoryMainWidget : public UR1UserWidget
{
	GENERATED_BODY()

protected:
	// 🌟 허공에 드롭했을 때 이벤트를 낚아챌 함수
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
