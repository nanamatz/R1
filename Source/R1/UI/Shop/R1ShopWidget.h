

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1ShopWidget.generated.h"

class UR1ShopGridWidget;
class UTextBlock;
class UR1ItemInstance;
class UButton;

/**
 * 
 */
UCLASS()
class R1_API UR1ShopWidget : public UR1UserWidget
{
	GENERATED_BODY()

public:
	void SetShopItems(const TArray<UR1ItemInstance*>& Items);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void UpdateGoldDisplay(int32 NewGold);

	UFUNCTION()
	void OnCloseButtonClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UR1ShopGridWidget> ShopGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_CurrentGold;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Close;
};
