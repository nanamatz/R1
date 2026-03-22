

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1InventoryItemTooltipWidget.generated.h"

class UTextBlock;
class UR1ItemInstance;
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
	void SetupTooltip(UR1ItemInstance* ItemInstance, bool bIsShopContext);

protected:
	UPROPERTY(meta = (BindWidget)) 
	TObjectPtr<UTextBlock> Text_ItemName;
	UPROPERTY(meta = (BindWidget)) 
	TObjectPtr<UTextBlock> Text_ItemType;
	UPROPERTY(meta = (BindWidget)) 
	TObjectPtr<UTextBlock> Text_Price;
};
