


#include "UI/Inventory/R1InventoryMainWidget.h"
#include "Item/R1DragDropOperation.h"
#include "Item/R1ItemInstance.h"
#include "Player/R1PlayerController.h"

bool UR1InventoryMainWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// 1. 드래그 정보가 우리가 만든 아이템 드래그 객체인지 확인
	if (UR1DragDropOperation* DragDropOp = Cast<UR1DragDropOperation>(InOperation))
	{
		// 2. 알맹이(아이템)가 잘 들어있는지 확인
		if (DragDropOp->ItemInstance)
		{
			// 3. 내 플레이어 컨트롤러를 가져와서 캐스팅
			if (AR1PlayerController* PC = Cast<AR1PlayerController>(GetOwningPlayer()))
			{
				// 🌟 4. 컨트롤러에게 월드에 버리라고 명령!
				PC->DropItemToWorld(DragDropOp->ItemInstance, DragDropOp->FromEquipmentSlot);

				UE_LOG(LogTemp, Warning, TEXT("[R1InventoryMainWidget] 인벤토리 밖으로 아이템을 던졌습니다!"));

				// 처리를 완료했으므로 true 반환 (이벤트 릴레이 종료)
				return true;
			}
		}
	}

	return false;
}
