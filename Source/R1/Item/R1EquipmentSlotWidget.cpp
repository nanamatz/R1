


#include "Item/R1EquipmentSlotWidget.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Item/R1DragDropOperation.h"
#include "Item/R1ItemInstance.h"
#include "Item/R1InventorySubsystem.h"

void UR1EquipmentSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (SizeBox_Root)
	{
		FVector2D UnitSlotSize = FVector2D(Item::UnitInventorySlotSize);	
		FIntPoint SlotSize = FIntPoint(1, 1);

		// 💡 부위에 따라 기본 빈 칸의 크기를 결정합니다!
		switch (EquipmentSlotType)
		{
		case ER1EquipmentSlot::Weapon:
			SlotSize = FIntPoint(2, 3); // 무기, 갑옷은 2x3
			break;
		case ER1EquipmentSlot::Helmet:
		case ER1EquipmentSlot::Boots:
		case ER1EquipmentSlot::Armor:
		case ER1EquipmentSlot::Glove:
			SlotSize = FIntPoint(2, 2); // 투구, 신발은 2x2
			break;
		case ER1EquipmentSlot::Ring:
			SlotSize = FIntPoint(1, 1); // 반지는 1x1
			break;
		}

		SizeBox_Root->SetWidthOverride(SlotSize.X * UnitSlotSize.X);
		SizeBox_Root->SetHeightOverride(SlotSize.Y * UnitSlotSize.Y);
	}
}

bool UR1EquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// 드래그 해온 데이터가 아이템인지 확인
	UR1DragDropOperation* DragDropOp = Cast<UR1DragDropOperation>(InOperation);
	if (!DragDropOp || !DragDropOp->ItemInstance)
	{
		return false;
	}

	// 💡 가장 중요한 검증: "가져온 아이템의 부위가 내 부위와 일치하는가?"
	if (DragDropOp->ItemInstance->EquipSlot != EquipmentSlotType)
	{
		UE_LOG(LogTemp, Warning, TEXT("착용할 수 없는 부위입니다!"));
		return false; // 부위가 다르면 드롭을 거부합니다 (튕겨나감)
	}

	UR1InventorySubsystem* InventorySubsystem = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (InventorySubsystem)
	{
		UR1ItemInstance* OldEquippedItem = EquippedItem;

		InventorySubsystem->RemoveItemFromGrid(DragDropOp->ItemInstance, DragDropOp->FromItemSlotPos);
		// 실제 장착 실행! (인벤토리에서 빠지고 Map에 들어감)
		if (OldEquippedItem != nullptr)
		{
			// 이미 장비가 있었다면 기존 장비를 인벤토리로 돌려보냄.
			InventorySubsystem->AddItemToGrid(OldEquippedItem, DragDropOp->FromItemSlotPos);

			// TODO: InventorySubsystem->UnequipItem(OldEquippedItem); // 서브시스템에 스탯 감소 등 장착 해제 로직이 있다면 여기서 호출
			UE_LOG(LogTemp, Warning, TEXT("기존 장비와 교체(Swap) 되었습니다!"));
		}
		InventorySubsystem->EquipItem(DragDropOp->ItemInstance);
		EquippedItem = DragDropOp->ItemInstance;
		// 아이템 아이콘 세팅 (ItemIcon 변수가 ItemInstance에 존재한다고 가정)
		
		if (EquippedItem->ItemIcon)
		{
			Image_ItemIcon->SetBrushFromTexture(EquippedItem->ItemIcon);
			Image_ItemIcon->SetRenderOpacity(1.0f);
		}

		InventorySubsystem->OnInventoryUpdated.Broadcast();

		return true;
	}

	return false;
}
