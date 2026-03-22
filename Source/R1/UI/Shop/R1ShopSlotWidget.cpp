#include "UI/Shop/R1ShopSlotWidget.h"
#include "Item/R1InventorySubsystem.h"
#include "Data/R1ItemAssetData.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UR1ShopSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Buy)
	{
		Button_Buy->OnClicked.AddDynamic(this, &UR1ShopSlotWidget::OnBuyButtonClicked);
	}
}

void UR1ShopSlotWidget::SetItem(UR1ItemAssetData* InItemData)
{
	CurrentItemData = InItemData;
	if (!CurrentItemData) return;

	if (Icon_Item)
	{
		Icon_Item->SetBrushFromTexture(CurrentItemData->ItemIcon);
	}

	if (Text_ItemName)
	{
		Text_ItemName->SetText(FText::FromName(CurrentItemData->ItemName));
	}

	if (Text_Price)
	{
		Text_Price->SetText(FText::AsNumber(CurrentItemData->BaseValue));
	}

	// 희귀도 텍스트 및 배경 색상 설정
	FText RarityText;
	FLinearColor RarityColor;

	switch (CurrentItemData->ItemRarity)
	{
	case EItemRarity::Common:
		RarityText = FText::FromString(TEXT("Common"));
		RarityColor = FLinearColor(0.8f, 0.8f, 0.8f);
		break;
	case EItemRarity::Uncommon:
		RarityText = FText::FromString(TEXT("Uncommon"));
		RarityColor = FLinearColor(0.1f, 0.8f, 0.1f);
		break;
	case EItemRarity::Rare:
		RarityText = FText::FromString(TEXT("Rare"));
		RarityColor = FLinearColor(0.0f, 0.5f, 1.0f);
		break;
	case EItemRarity::Epic:
		RarityText = FText::FromString(TEXT("Epic"));
		RarityColor = FLinearColor(0.7f, 0.0f, 1.0f);
		break;
	case EItemRarity::Legendary:
		RarityText = FText::FromString(TEXT("Legendary"));
		RarityColor = FLinearColor(1.0f, 0.5f, 0.0f);
		break;
	default:
		RarityText = FText::FromString(TEXT("Common"));
		RarityColor = FLinearColor(0.8f, 0.8f, 0.8f);
		break;
	}

	if (Text_Rarity)
	{
		Text_Rarity->SetText(RarityText);
		Text_Rarity->SetColorAndOpacity(RarityColor);
	}

	if (Image_RarityBG)
	{
		Image_RarityBG->SetColorAndOpacity(RarityColor);
	}
}

void UR1ShopSlotWidget::OnBuyButtonClicked()
{
	if (!CurrentItemData) return;

	if (UWorld* World = GetWorld())
	{
		if (UR1InventorySubsystem* InventorySubsystem = World->GetSubsystem<UR1InventorySubsystem>())
		{
			InventorySubsystem->BuyItem(CurrentItemData, CurrentItemData->ItemRarity);
		}
	}
}
