

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1Define.h"
#include "R1InventoryItemTooltipWidget.generated.h"

class UTextBlock;
class UR1ItemInstance;
struct FGameplayTag;
/**
 * 
 */
UCLASS()
class R1_API UR1InventoryItemTooltipWidget : public UR1UserWidget
{
	GENERATED_BODY()

public:
	// bIsShopContext가 true면 '구매가(BaseValue)', false면 '판매가(GetSellPrice)'를 표시합니다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetupTooltip(UR1ItemInstance* ItemInstance, bool bIsShopContext, bool bIsEquipped = false);
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_IsEquipped;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ItemName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Rarity;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ItemType;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_EquipSlotType;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Stats;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Description;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Price;
private:
	// Enum을 텍스트로 변환하는 헬퍼 함수들
	FText GetItemTypeText(ER1ItemType ItemType);
	FText GetEquipSlotText(ER1EquipmentSlot EquipSlot);
	FSlateColor GetRarityColor(EItemRarity Rarity);
	FText GetRarityText(EItemRarity Rarity);

	FString GetStatNameByTag(const FGameplayTag& Tag);
};
