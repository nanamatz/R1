#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1ShopGridWidget.generated.h"

class UR1ItemInstance;
class UUniformGridPanel;
class UCanvasPanel;
class UR1InventroySlotWidget;
class UR1ShopEntryWidget;

/**
 * 
 */
UCLASS()
class R1_API UR1ShopGridWidget : public UR1UserWidget
{
	GENERATED_BODY()

public:
	void InitShopGrid(const TArray<UR1ItemInstance*>& Items);

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	int32 X_COUNT = 6;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	int32 Y_COUNT = 3;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UR1InventroySlotWidget> SlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UR1ShopEntryWidget> EntryWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel_Slots;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel_Entries;

private:
	UPROPERTY()
	TArray<TObjectPtr<UR1InventroySlotWidget>> SlotWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UR1ShopEntryWidget>> EntryWidgets;

	bool CanAddItemAt(const FIntPoint& ItemSize, const FIntPoint& TargetPos, const TArray<UR1ItemInstance*>& GridMap);
};
