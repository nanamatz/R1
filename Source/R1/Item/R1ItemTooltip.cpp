


#include "Item/R1ItemTooltip.h"
#include "Components/TextBlock.h"
#include "R1Define.h"
#include "Item/R1InventorySubsystem.h"
#include "Library/R1ItemFunctionLibrary.h"

void UR1ItemTooltip::SetItemInfo(const FText& InName, EItemRarity InRarity,int32 ItemCount, ER1ItemType InItemType, int32 BaseValue, bool bIsPlayerInventoryItem)
{
	if (!Text_ItemName) return;

	FString DisplayName = InName.ToString();

	if ((InItemType == ER1ItemType::Consumable || InItemType == ER1ItemType::Key) && ItemCount >= 2)
	{
		DisplayName = FString::Printf(TEXT("%s (%d)"), *DisplayName, ItemCount);
	}

	// 1. 이름 텍스트 적용
	Text_ItemName->SetText(FText::FromString(DisplayName));
	Text_ItemName->SetColorAndOpacity(UR1ItemFunctionLibrary::GetRarityColor(InRarity));
}
