

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "R1Define.h"
#include "R1DragDropOperation.generated.h"

class UR1ItemInstance;
/**
 * 
 */
UCLASS()
class R1_API UR1DragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	UR1DragDropOperation(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	FIntPoint FromItemSlotPos = FIntPoint::ZeroValue;

	ER1EquipmentSlot FromEquipmentSlot = ER1EquipmentSlot::None;

public:
	UPROPERTY()
	TObjectPtr<UR1ItemInstance> ItemInstance;

	FVector2D DeltaWidgetPos = FVector2D::ZeroVector;
};
