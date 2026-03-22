


#include "Item/R1InventoryItemTooltipWidget.h"
#include "Components/TextBlock.h"
#include "Item/R1ItemInstance.h"
#include "Data/R1ItemAssetData.h"

void UR1InventoryItemTooltipWidget::SetupTooltip(UR1ItemInstance* ItemInstance, bool bIsShopContext)
{
	if (!ItemInstance || !ItemInstance->GetItemData()) return;

	UR1ItemAssetData* Data = ItemInstance->GetItemData();

	// 1. 기본 텍스트 세팅
	if (Text_ItemName) Text_ItemName->SetText(FText::FromName(Data->ItemName));
	// (타입 등 추가 정보 세팅 생략)

	// 2. 💰 컨텍스트에 따른 가격 표시 로직
	if (Text_Price)
	{
		int32 DisplayPrice = 0;
		FString PricePrefix = TEXT("");

		if (bIsShopContext)
		{
			// 상점에 있는 아이템 (구매가)
			DisplayPrice = Data->BaseValue * ItemInstance->ItemCount;
			PricePrefix = TEXT("구매가: ");
		}
		else
		{
			// 내 인벤토리에 있는 아이템 (판매가: 30% 감소)
			DisplayPrice = FMath::Max(1, FMath::FloorToInt(Data->BaseValue * 0.7f)) * ItemInstance->ItemCount;
			PricePrefix = TEXT("판매가: ");
		}

		FString FinalPriceText = FString::Printf(TEXT("%s%d 골드"), *PricePrefix, DisplayPrice);
		Text_Price->SetText(FText::FromString(FinalPriceText));
	}
}
