

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1ShopWidget.generated.h"

class UR1ShopSlotWidget;
class UCommonTextBlock;
class UR1ItemAssetData;

/**
 * 
 */
UCLASS()
class R1_API UR1ShopWidget : public UR1UserWidget
{
	GENERATED_BODY()

public:
	void SetShopItems(const TArray<UR1ItemAssetData*>& Items);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void UpdateGoldDisplay(int32 NewGold);

	UFUNCTION()
	void OnCloseButtonClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UR1ShopSlotWidget> ShopSlot_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UR1ShopSlotWidget> ShopSlot_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UR1ShopSlotWidget> ShopSlot_2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_CurrentGold;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Close;
};
