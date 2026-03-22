
#include "UI/Shop/R1ShopWidget.h"
#include "UI/Shop/R1ShopSlotWidget.h"
#include "Item/R1InventorySubsystem.h"
#include "CommonTextBlock.h"

void UR1ShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWorld* World = GetWorld())
	{
		if (UR1InventorySubsystem* InventorySubsystem = World->GetSubsystem<UR1InventorySubsystem>())
		{
			// 초기 골드 표시
			UpdateGoldDisplay(InventorySubsystem->GetGold());

			// 골드 변경 시 콜백 등록
			InventorySubsystem->OnGoldChanged.AddDynamic(this, &UR1ShopWidget::UpdateGoldDisplay);
		}
	}
}

void UR1ShopWidget::SetShopItems(const TArray<UR1ItemAssetData*>& Items)
{
	if (Items.IsValidIndex(0) && ShopSlot_0)
	{
		ShopSlot_0->SetItem(Items[0]);
	}

	if (Items.IsValidIndex(1) && ShopSlot_1)
	{
		ShopSlot_1->SetItem(Items[1]);
	}

	if (Items.IsValidIndex(2) && ShopSlot_2)
	{
		ShopSlot_2->SetItem(Items[2]);
	}
}

void UR1ShopWidget::UpdateGoldDisplay(int32 NewGold)
{
	if (Text_CurrentGold)
	{
		Text_CurrentGold->SetText(FText::AsNumber(NewGold));
	}
}
