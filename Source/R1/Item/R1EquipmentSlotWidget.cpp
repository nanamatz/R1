


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

	UE_LOG(LogTemp, Warning, TEXT("장착 성공! 부위: %d"), (int32)EquipmentSlotType);

	UR1InventorySubsystem* InventorySubsystem = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (InventorySubsystem)
	{
		// 실제 장착 실행! (인벤토리에서 빠지고 Map에 들어감)
		InventorySubsystem->EquipItem(DragDropOp->ItemInstance);

		// 3. 내 UI 슬롯 갱신
		EquippedItem = DragDropOp->ItemInstance;

		// 아이템 아이콘 세팅 (ItemIcon 변수가 ItemInstance에 존재한다고 가정)
		
		if (EquippedItem->ItemIcon)
		{
			Image_ItemIcon->SetBrushFromTexture(EquippedItem->ItemIcon);
			Image_ItemIcon->SetRenderOpacity(1.0f); // 숨겨뒀던 아이콘 켜기
		}
		
	}

	return true; // true를 반환하면 드롭이 성공적으로 끝났음을 의미
}
