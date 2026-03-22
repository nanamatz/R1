
#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1SellQuantityPopup.generated.h"

class UR1ItemInstance;
class UImage;
class UCommonTextBlock;
class UCommonButtonBase;

/**
 * 
 */
UCLASS()
class R1_API UR1SellQuantityPopup : public UR1UserWidget
{
	GENERATED_BODY()

public:
	void SetItem(UR1ItemInstance* InItem);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnPlusButtonClicked();

	UFUNCTION()
	void OnMinusButtonClicked();

	UFUNCTION()
	void OnConfirmButtonClicked();

	UFUNCTION()
	void OnCancelButtonClicked();

	void UpdateQuantityDisplay();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Icon_Item;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_ItemName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Quantity;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Plus;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Minus;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Confirm;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Cancel;

private:
	UPROPERTY()
	TObjectPtr<UR1ItemInstance> TargetItem;

	int32 SelectedQuantity = 1;
};
