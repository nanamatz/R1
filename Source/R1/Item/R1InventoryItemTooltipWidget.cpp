


#include "Item/R1InventoryItemTooltipWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Item/R1ItemInstance.h"
#include "Data/R1ItemAssetData.h"
#include "R1GameplayTags.h"
#include "Library/R1ItemFunctionLibrary.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "UI/Inventory/Item/R1StatRowWidget.h"
#include "System/R1StatUISettings.h"

void UR1InventoryItemTooltipWidget::SetupTooltip(UR1ItemInstance* ItemInstance, bool bIsShopContext, bool bIsEquipped)
{
	if (!ItemInstance || !ItemInstance->GetItemData()) return;

	UR1ItemAssetData* Data = ItemInstance->GetItemData();
	if (Text_IsEquipped)
	{
		if (bIsEquipped)
		{
			Text_IsEquipped->SetText(FText::FromString(FString::Printf(TEXT("(Equipped)"))));
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

	if (VerticalBox_Stats)
	{
		VerticalBox_Stats->ClearChildren(); // 이전 데이터 비우기

		if (Data->StatModifiers.Num() > 0 && StatRowClass)
		{
			// 프로젝트 세팅 데이터 로드
			const UR1StatUISettings* StatSettings = GetDefault<UR1StatUISettings>();

			for (const auto& ModifierPair : Data->StatModifiers)
			{
				FGameplayTag Tag = ModifierPair.Key;
				float StatValue = ModifierPair.Value;

				FString StatNameStr = TEXT("None");
				UTexture2D* StatIcon = nullptr;

				// 태그 검색 및 매핑
				if (const FR1StatUIInfo* FoundInfo = StatSettings->StatUIMap.Find(Tag))
				{
					StatNameStr = FoundInfo->StatName.ToString();
					StatIcon = FoundInfo->StatIcon;
				}
				else
				{
					StatNameStr = Tag.GetTagName().ToString().Replace(TEXT("Data.Attribute."), TEXT(""));
				}

				FString ValueString = TEXT("");

				// 🌟 Ability면 수치 텍스트를 비워둡니다.
				if (!Tag.GetTagName().ToString().Contains(TEXT("Ability")))
				{
					if (Tag.GetTagName().ToString().Contains(TEXT("Multiplier")))
					{
						int32 PercentValue = FMath::RoundToInt(StatValue * 100.0f);
						ValueString = FString::Printf(TEXT("%d%%"), PercentValue);
					}
					else
					{
						int32 IntValue = FMath::RoundToInt(StatValue);
						ValueString = FString::Printf(TEXT("%d"), IntValue);
					}
				}

				// Row 위젯 생성 및 수직 박스에 추가
				UR1StatRowWidget* StatRow = CreateWidget<UR1StatRowWidget>(this, StatRowClass);
				if (StatRow)
				{
					StatRow->InitStatRow(StatIcon, StatNameStr,ValueString);
					VerticalBox_Stats->AddChildToVerticalBox(StatRow);
				}
			}

			VerticalBox_Stats->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			VerticalBox_Stats->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	if (Text_Skill)
	{
		if (Data->GrantedAbilities.Num() > 0)
		{
			FString AbilityString = TEXT("");

			// 배열에 들어있는 클래스(TSubclassOf)들을 순회합니다.
			for (const TSubclassOf<UR1GameplayAbility>& AbilityClass : Data->GrantedAbilities)
			{
				if (AbilityClass)
				{
					// 🌟 핵심: 클래스를 스폰하지 않고, 메모리에 떠 있는 기본값(CDO) 껍데기만 쏙 가져옵니다!
					const UR1GameplayAbility* DefaultAbility = AbilityClass.GetDefaultObject();

					// 설명이 적혀있을 때만 추가
					if (DefaultAbility && !DefaultAbility->AbilityDescription.IsEmpty())
					{
						AbilityString += FString::Printf(TEXT("%s\n"), *DefaultAbility->AbilityDescription.ToString());
					}
				}
			}

			// 맨 마지막 줄바꿈 제거
			AbilityString = AbilityString.TrimEnd();

			Text_Skill->SetText(FText::FromString(AbilityString));
			Text_Skill->SetJustification(ETextJustify::Center); 
			Text_Skill->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Text_Skill->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 7. 가격 표시 (상점과 인벤토리의 문맥 차이 적용)
	if (Text_Price)
	{
		int32 DisplayPrice = bIsShopContext ? (Data->BaseValue * ItemInstance->ItemCount) : (FMath::Max(1, FMath::FloorToInt(Data->BaseValue * 0.7f)) * ItemInstance->ItemCount);

		FString FinalPriceText = FString::Printf(TEXT("%d"), DisplayPrice);

		Text_Price->SetText(FText::FromString(FinalPriceText));
	}
}

FString UR1InventoryItemTooltipWidget::GetStatNameByTag(const FGameplayTag& Tag)
{
	// 공통 속성
	if (Tag == R1GameplayTags::Data_Attribute_MaxHealth)			return TEXT("Health");
	if (Tag == R1GameplayTags::Data_Attribute_HealthRegeneration)	return TEXT("Health Regen");
	if (Tag == R1GameplayTags::Data_Attribute_AttackRange)			return TEXT("Attack Range");
	if (Tag == R1GameplayTags::Data_Attribute_AttackRadius)			return TEXT("Attack Radius");
	if (Tag == R1GameplayTags::Data_Attribute_AttackSpeed)			return TEXT("Attack Speed");
	if (Tag == R1GameplayTags::Data_Attribute_MoveSpeed)			return TEXT("Move Speed");

	// 플레이어 전용 속성
	if (Tag == R1GameplayTags::Data_Attribute_MaxMana)				return TEXT("Mana");
	if (Tag == R1GameplayTags::Data_Attribute_ManaRegeneration)		return TEXT("Mana Regen");
	if (Tag == R1GameplayTags::Data_Attribute_WeaponDamage)			return TEXT("Damage");
	if (Tag == R1GameplayTags::Data_Attribute_EquipDefence)			return TEXT("Defense");

	// Multiplier 계열 (퍼센트 적용 대상)
	if (Tag == R1GameplayTags::Data_Attribute_DamageMultiplier)		return TEXT("Damage Multiplier");
	if (Tag == R1GameplayTags::Data_Attribute_DefenceMultiplier)	return TEXT("Defense Multiplier");

	// 예외 처리 방어 코드
	FString FallbackName = Tag.GetTagName().ToString();
	FallbackName = FallbackName.Replace(TEXT("Data.Attribute."), TEXT(""));
	return FallbackName;
}
