#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1ShopEntryWidget.generated.h"

class UImage;
class UR1ItemInstance;

/**
 * 
 */
UCLASS()
class R1_API UR1ShopEntryWidget : public UR1UserWidget
{
	GENERATED_BODY()

public:
	void Init(UR1ItemInstance* InItemInstance);

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Icon_Item;

	UPROPERTY(EditDefaultsOnly, Category = "Tooltip")
	TSubclassOf<class UR1ItemTooltip> TooltipClass;

private:
	UPROPERTY()
	TObjectPtr<UR1ItemInstance> ItemInstance;

	UPROPERTY()
	TObjectPtr<class UR1ItemTooltip> TooltipWidget;
};
