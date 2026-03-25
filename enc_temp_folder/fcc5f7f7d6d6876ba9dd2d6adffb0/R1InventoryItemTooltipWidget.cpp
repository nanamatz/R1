


#include "Item/R1InventoryItemTooltipWidget.h"
#include "Components/TextBlock.h"
#include "Item/R1ItemInstance.h"
#include "Data/R1ItemAssetData.h"
#include "R1GameplayTags.h"
#include "Library/R1ItemFunctionLibrary.h"

void UR1InventoryItemTooltipWidget::SetupTooltip(UR1ItemInstance* ItemInstance, bool bIsShopContext, bool bIsEquipped)
{
	if (!ItemInstance || !ItemInstance->GetItemData()) return;

	UR1ItemAssetData* Data = ItemInstance->GetItemData();
	if (Text_IsEquipped)
	{
		if (bIsEquipped)
		{
			Text_IsEquipped->SetText(FText::FromString(FString::Printf(TEXT("(장착중)"))));
		}
		else
		{
			Text_IsEquipped->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	// 1. 아이템 명 & 색상
	if (Text_ItemName)
	{
		FString DisplayName = Data->ItemName.ToString();

		Text_ItemName->SetText(FText::FromString(DisplayName));
		Text_ItemName->SetColorAndOpacity(UR1ItemFunctionLibrary::GetRarityColor(ItemInstance->ItemRarity));
	}

	// 2. 희귀도 텍스트 & 색상
	if (Text_Rarity)
	{
		Text_Rarity->SetText(UR1ItemFunctionLibrary::GetRarityText(ItemInstance->ItemRarity));
		Text_Rarity->SetColorAndOpacity(UR1ItemFunctionLibrary::GetRarityColor(ItemInstance->ItemRarity));
	}

	// 3. 아이템 분류 (장비, 소모품 등)
	if (Text_ItemType)
	{
		Text_ItemType->SetText(UR1ItemFunctionLibrary::GetItemTypeText(Data->ItemType));
	}

	// 4. 장비 부위 (장비가 아니면 레이아웃에서 압축하여 숨김)
	if (Text_EquipSlotType)
	{
		if (Data->ItemType == ER1ItemType::Equipment && Data->EquipSlots.Num() > 0)
		{
			Text_EquipSlotType->SetText(UR1ItemFunctionLibrary::GetEquipSlotText(Data->EquipSlots[0]));
			Text_EquipSlotType->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Text_EquipSlotType->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 5. 아이템 스펙 (StatModifiers TMap 순회, 없으면 압축)
	if (Text_Stats)
	{
		if (Data->StatModifiers.Num() > 0)
		{
			FString StatsString = TEXT("");

			for (const auto& ModifierPair : Data->StatModifiers)
			{
				FGameplayTag Tag = ModifierPair.Key;
				float StatValue = ModifierPair.Value;

				FString StatName = GetStatNameByTag(Tag);
				FString DisplayString;

				// Multiplier(증폭) 태그는 퍼센트로 처리하는 디테일
				if (Tag.GetTagName().ToString().Contains(TEXT("Multiplier")))
				{
					int32 PercentValue = FMath::RoundToInt(StatValue * 100.0f);
					FString Sign = (PercentValue > 0) ? TEXT("+") : TEXT("");
					DisplayString = FString::Printf(TEXT("%s%d%%"), *Sign, PercentValue);
				}
				else
				{
					int32 IntValue = FMath::RoundToInt(StatValue);
					FString Sign = (IntValue > 0) ? TEXT("+") : TEXT("");
					DisplayString = FString::Printf(TEXT("%s%d"), *Sign, IntValue);
				}

				// 예: "최대 체력 +100\n"
				StatsString += FString::Printf(TEXT("%s %s\n"), *StatName, *DisplayString);
			}

			// 마지막 줄바꿈(\n) 제거
			StatsString = StatsString.TrimEnd();

			Text_Stats->SetText(FText::FromString(StatsString));
			Text_Stats->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Text_Stats->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 6. 아이템 설명 (데이터가 없으면 압축)
	if (Text_Description)
	{
		if (!Data->ItemDescription.IsEmpty())
		{
			Text_Description->SetText(Data->ItemDescription);
			Text_Description->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Text_Description->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 7. 가격 표시 (상점과 인벤토리의 문맥 차이 적용)
	if (Text_Price)
	{
		int32 DisplayPrice = bIsShopContext ? (Data->BaseValue * ItemInstance->ItemCount) : (FMath::Max(1, FMath::FloorToInt(Data->BaseValue * 0.7f)) * ItemInstance->ItemCount);

		FString ContextPrefix = bIsShopContext ? TEXT("가격: ") : TEXT("가격: ");
		FString FinalPriceText = FString::Printf(TEXT("%s%d"), *ContextPrefix, DisplayPrice);

		Text_Price->SetText(FText::FromString(FinalPriceText));
	}
}

FText UR1InventoryItemTooltipWidget::GetItemTypeText(ER1ItemType ItemType)
{
	switch (ItemType)
	{
	case ER1ItemType::Equipment: return FText::FromString(TEXT("장비: "));
	case ER1ItemType::Consumable: return FText::FromString(TEXT("소모품"));
	case ER1ItemType::Material: return FText::FromString(TEXT("재료"));
	case ER1ItemType::Key: return FText::FromString(TEXT("열쇠"));
	}
	return FText::GetEmpty();
}

FText UR1InventoryItemTooltipWidget::GetEquipSlotText(ER1EquipmentSlot EquipSlot)
{
	switch (EquipSlot)
	{
	case ER1EquipmentSlot::Weapon: return FText::FromString(TEXT("무기"));
	case ER1EquipmentSlot::Helmet: return FText::FromString(TEXT("머리"));
	case ER1EquipmentSlot::Armor: return FText::FromString(TEXT("갑옷"));
	case ER1EquipmentSlot::Glove: return FText::FromString(TEXT("장갑"));
	case ER1EquipmentSlot::Ring1:
	case ER1EquipmentSlot::Ring2: return FText::FromString(TEXT("반지"));
	case ER1EquipmentSlot::Boots: return FText::FromString(TEXT("신발"));
	}
	return FText::GetEmpty();
}

FSlateColor UR1InventoryItemTooltipWidget::GetRarityColor(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Common: return FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f));
	case EItemRarity::Uncommon: return FSlateColor(FLinearColor(0.1f, 0.8f, 0.1f));
	case EItemRarity::Rare: return FSlateColor(FLinearColor(0.0f, 0.5f, 1.0f));
	case EItemRarity::Epic: return FSlateColor(FLinearColor(0.7f, 0.0f, 1.0f));
	case EItemRarity::Legendary: return FSlateColor(FLinearColor(1.0f, 0.5f, 0.0f));
	}
	return FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f));
}

FText UR1InventoryItemTooltipWidget::GetRarityText(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Common: return FText::FromString(TEXT("일반"));
	case EItemRarity::Uncommon: return FText::FromString(TEXT("고급"));
	case EItemRarity::Rare: return FText::FromString(TEXT("희귀"));
	case EItemRarity::Epic: return FText::FromString(TEXT("영웅"));
	case EItemRarity::Legendary: return FText::FromString(TEXT("전설"));
	}
	return FText::GetEmpty();
}

FString UR1InventoryItemTooltipWidget::GetStatNameByTag(const FGameplayTag& Tag)
{
	// 공통 속성
	if (Tag == R1GameplayTags::Data_Attribute_MaxHealth)			return TEXT("최대 체력");
	if (Tag == R1GameplayTags::Data_Attribute_HealthRegeneration)	return TEXT("체력 재생");
	if (Tag == R1GameplayTags::Data_Attribute_AttackRange)			return TEXT("공격 사거리");
	if (Tag == R1GameplayTags::Data_Attribute_AttackRadius)			return TEXT("공격 범위");
	if (Tag == R1GameplayTags::Data_Attribute_AttackSpeed)			return TEXT("공격 속도");
	if (Tag == R1GameplayTags::Data_Attribute_MoveSpeed)			return TEXT("이동 속도");

	// 플레이어 전용 속성
	if (Tag == R1GameplayTags::Data_Attribute_MaxMana)				return TEXT("최대 마나");
	if (Tag == R1GameplayTags::Data_Attribute_ManaRegeneration)		return TEXT("마나 재생");
	if (Tag == R1GameplayTags::Data_Attribute_WeaponDamage)			return TEXT("공격력");
	if (Tag == R1GameplayTags::Data_Attribute_EquipDefence)			return TEXT("방어력");

	// Multiplier 계열 (퍼센트 적용 대상)
	if (Tag == R1GameplayTags::Data_Attribute_DamageMultiplier)		return TEXT("피해량 증폭");
	if (Tag == R1GameplayTags::Data_Attribute_DefenceMultiplier)	return TEXT("방어력 증폭");

	// 예외 처리 방어 코드
	FString FallbackName = Tag.GetTagName().ToString();
	FallbackName = FallbackName.Replace(TEXT("Data.Attribute."), TEXT(""));
	return FallbackName;
}
