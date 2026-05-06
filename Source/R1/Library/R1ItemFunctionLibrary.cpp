


#include "Library/R1ItemFunctionLibrary.h"

FSlateColor UR1ItemFunctionLibrary::GetRarityColor(EItemRarity Rarity)
{
	// 💡 "쨍한 원색"의 채도(S)와 명도(V)를 낮춰 눈이 편안한 고급스러운 톤으로 조정했습니다.
	switch (Rarity)
	{
	case EItemRarity::Common:
		return FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)); // 차분한 회색
	case EItemRarity::Uncommon:
		return FSlateColor(FLinearColor(0.3f, 0.6f, 0.3f)); // 부드러운 초록
	case EItemRarity::Rare:
		return FSlateColor(FLinearColor(0.2f, 0.4f, 0.8f)); // 차분한 파랑
	case EItemRarity::Epic:
		return FSlateColor(FLinearColor(0.6f, 0.3f, 0.7f)); // 우아한 보라
	case EItemRarity::Legendary:
		return FSlateColor(FLinearColor(1.0f, 0.5f, 0.1f)); // 묵직한 금색
	}
	return FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f));
}

FText UR1ItemFunctionLibrary::GetRarityText(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Common: return FText::FromString(TEXT("Common"));
	case EItemRarity::Uncommon: return FText::FromString(TEXT("Uncommon"));
	case EItemRarity::Rare: return FText::FromString(TEXT("Rare"));
	case EItemRarity::Epic: return FText::FromString(TEXT("Epic"));
	case EItemRarity::Legendary: return FText::FromString(TEXT("Legendary"));
	}
	return FText::GetEmpty();
}

FText UR1ItemFunctionLibrary::GetItemTypeText(ER1ItemType ItemType)
{
	switch (ItemType)
	{
	case ER1ItemType::Equipment: return FText::FromString(TEXT("Equipment: "));
	case ER1ItemType::Consumable: return FText::FromString(TEXT("Consumable"));
	case ER1ItemType::Material: return FText::FromString(TEXT("Material"));
	case ER1ItemType::Key: return FText::FromString(TEXT("Key"));
	}
	return FText::GetEmpty();
}

FText UR1ItemFunctionLibrary::GetEquipSlotText(ER1EquipmentSlot EquipSlot)
{
	switch (EquipSlot)
	{
	case ER1EquipmentSlot::Weapon: return FText::FromString(TEXT("Weapon"));
	case ER1EquipmentSlot::Helmet: return FText::FromString(TEXT("Helm"));
	case ER1EquipmentSlot::Armor: return FText::FromString(TEXT("Armor"));
	case ER1EquipmentSlot::Glove: return FText::FromString(TEXT("Glove"));
	case ER1EquipmentSlot::Ring1:
	case ER1EquipmentSlot::Ring2: return FText::FromString(TEXT("Ring"));
	case ER1EquipmentSlot::Boots: return FText::FromString(TEXT("Boots"));
	}
	return FText::GetEmpty();
}