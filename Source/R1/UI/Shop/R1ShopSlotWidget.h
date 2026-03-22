

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1ShopSlotWidget.generated.h"

class UR1ItemAssetData;
class UImage;
class UTextBlock;
class UButton;

/**
 * 
 */
UCLASS()
class R1_API UR1ShopSlotWidget : public UR1UserWidget
{
	GENERATED_BODY()

public:
	void SetItem(UR1ItemAssetData* InItemData);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnBuyButtonClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Icon_Item;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ItemName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Rarity;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_RarityBG;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Price;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Buy;

private:
	UPROPERTY()
	TObjectPtr<UR1ItemAssetData> CurrentItemData;
};
