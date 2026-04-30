#include "UI/Inventory/R1InventoryMainWidget.h"
#include "Item/R1DragDropOperation.h"
#include "Item/R1ItemInstance.h"
#include "Player/R1PlayerController.h"
#include "Item/R1InventorySubsystem.h"


bool UR1InventoryMainWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{	
    if (UR1DragDropOperation* DragDropOp = Cast<UR1DragDropOperation>(InOperation))
    {
        if (DragDropOp->ItemInstance)
        {
            if (AR1PlayerController* PC = Cast<AR1PlayerController>(GetOwningPlayer()))
            {
                // 🌟 월드에 아이템을 드랍합니다.
                PC->DropItemToWorld(DragDropOp->ItemInstance, DragDropOp->FromEquipmentSlot);

                UE_LOG(LogTemp, Warning, TEXT("[R1UserWidget] UI 밖(월드)에 아이템을 던졌습니다!"));
                return true;
            }
        }
    }

    return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

