#include "UI/Shop/R1ShopEntryWidget.h"
#include "Item/R1ItemInstance.h"
#include "Item/R1InventorySubsystem.h"
#include "Components/Image.h"
#include "Item/R1ItemTooltip.h"

void UR1ShopEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UR1ShopEntryWidget::Init(UR1ItemInstance* InItemInstance)
{
	ItemInstance = InItemInstance;

	if (ItemInstance && Icon_Item)
	{
		Icon_Item->SetBrushFromTexture(ItemInstance->GetItemIcon());
	}
}

FReply UR1ShopEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	// 구매 로직
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton || InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (ItemInstance)
		{
			if (UWorld* World = GetWorld())
			{
				if (UR1InventorySubsystem* InventorySubsystem = World->GetSubsystem<UR1InventorySubsystem>())
				{
					// 구매 성공 시 아이템을 상점에서 제거하는 처리는 R1ShopGridWidget에서 하거나 여기에서 숨김 처리 등 할 수 있음
					if (InventorySubsystem->BuyItem(ItemInstance->GetItemData(), ItemInstance->ItemRarity, ItemInstance->ItemCount))
					{
						// 상점에서 아이템 구매 성공 (삭제)
						SetVisibility(ESlateVisibility::Collapsed);
					}
				}
			}
		}
		
		return FReply::Handled();
	}

	return Reply.NativeReply;
}

void UR1ShopEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (ItemInstance && TooltipClass && !TooltipWidget)
	{
		TooltipWidget = CreateWidget<UR1ItemTooltip>(GetOwningPlayer(), TooltipClass);
		if (TooltipWidget)
		{
			TooltipWidget->SetItemInfo(
				FText::FromName(ItemInstance->GetItemData()->ItemName),
				ItemInstance->ItemRarity,
				ItemInstance->ItemCount,
				ItemInstance->GetItemData()->ItemType,
				ItemInstance->GetItemData()->BaseValue,
				false // 샵의 아이템이므로 할인 없음
			);
			SetToolTip(TooltipWidget);
		}
	}
}

void UR1ShopEntryWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	
	// Tooltip은 UE의 Tooltip 시스템에 의해 자동으로 숨겨집니다.
}
