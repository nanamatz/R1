
#include "UI/Shop/R1SellQuantityPopup.h"
#include "Item/R1ItemInstance.h"
#include "Item/R1InventorySubsystem.h"
#include "Components/Image.h"
#include "CommonTextBlock.h"
#include "CommonButtonBase.h"

void UR1SellQuantityPopup::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Plus)
	{
		Button_Plus->OnClicked().AddUObject(this, &UR1SellQuantityPopup::OnPlusButtonClicked);
	}

	if (Button_Minus)
	{
		Button_Minus->OnClicked().AddUObject(this, &UR1SellQuantityPopup::OnMinusButtonClicked);
	}

	if (Button_Confirm)
	{
		Button_Confirm->OnClicked().AddUObject(this, &UR1SellQuantityPopup::OnConfirmButtonClicked);
	}

	if (Button_Cancel)
	{
		Button_Cancel->OnClicked().AddUObject(this, &UR1SellQuantityPopup::OnCancelButtonClicked);
	}
}

void UR1SellQuantityPopup::SetItem(UR1ItemInstance* InItem)
{
	TargetItem = InItem;
	if (!TargetItem) return;

	SelectedQuantity = 1;

	if (Icon_Item)
	{
		Icon_Item->SetBrushFromTexture(TargetItem->GetItemIcon());
	}

	if (Text_ItemName)
	{
		Text_ItemName->SetText(FText::FromName(TargetItem->GetItemData()->ItemName));
	}

	UpdateQuantityDisplay();
}

void UR1SellQuantityPopup::OnPlusButtonClicked()
{
	if (!TargetItem) return;

	SelectedQuantity = FMath::Min(SelectedQuantity + 1, TargetItem->ItemCount);
	UpdateQuantityDisplay();
}

void UR1SellQuantityPopup::OnMinusButtonClicked()
{
	SelectedQuantity = FMath::Max(SelectedQuantity - 1, 1);
	UpdateQuantityDisplay();
}

void UR1SellQuantityPopup::OnConfirmButtonClicked()
{
	if (TargetItem)
	{
		if (UWorld* World = GetWorld())
		{
			if (UR1InventorySubsystem* InventorySubsystem = World->GetSubsystem<UR1InventorySubsystem>())
			{
				InventorySubsystem->SellItem(TargetItem, SelectedQuantity);
			}
		}
	}

	RemoveFromParent();
}

void UR1SellQuantityPopup::OnCancelButtonClicked()
{
	RemoveFromParent();
}

void UR1SellQuantityPopup::UpdateQuantityDisplay()
{
	if (Text_Quantity)
	{
		Text_Quantity->SetText(FText::AsNumber(SelectedQuantity));
	}
}
