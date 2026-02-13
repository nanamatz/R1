

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1InventorySlotsWidget.generated.h"

class UR1InventroySlotWidget;
class UUniformGridPanel;
class UR1InventoryEntryWidget;
class UCanvasPanel;
class UR1ItemInstance;
/**
 * 
 */
UCLASS()
class R1_API UR1InventorySlotsWidget : public UR1UserWidget
{
	GENERATED_BODY()
	
public:
	UR1InventorySlotsWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	void OnInventoryEntryChanged(const FIntPoint& InItemSlotPos, TObjectPtr<UR1ItemInstance> Item);

	virtual void NativeConstruct() override;

	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,UDragDropOperation* InOperation);

private:
	void FinishDrag();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UR1InventroySlotWidget> SlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UR1InventoryEntryWidget> EntryWidgetClass;

	//UPROPERTY()
	//TSubclassOf<UR1InventorySlotsWidget> SlotWidgetClass;

	UPROPERTY()
	TArray<TObjectPtr<UR1InventroySlotWidget>> SlotWidgets;

	//UPROPERTY()
	//TSubclassOf<UR1InventoryEntryWidget> EntryWidgetClass;

	UPROPERTY()
	TArray<TObjectPtr<UR1InventoryEntryWidget>> EntryWidgets;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel_Slots;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel_Entries;

private:
	FIntPoint PreDragOverSlotPos = FIntPoint(-1, -1);

	const int X_COUNT = 10;
	const int Y_COUNT = 5;
};
