

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1ShopSlotsWidget.generated.h"

class UR1InventroySlotWidget;
class UUniformGridPanel;
class UR1InventoryEntryWidget;
class UCanvasPanel;
class UR1ItemInstance;
class UDragDropOperation;
class AR1MerchantNPC;
/**
 * 
 */
UCLASS()
class R1_API UR1ShopSlotsWidget : public UR1UserWidget
{
	GENERATED_BODY()
public:
	UR1ShopSlotsWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 상점 NPC와 연결하여 초기 3개의 아이템을 세팅합니다.
	UFUNCTION(BlueprintCallable, Category = "ShopUI")
	void InitShopGrid(AR1MerchantNPC* InNPC);

protected:
	virtual void NativeConstruct() override;

	// 드래그 중인 아이템이 상점 위를 지날 때 (시각적 피드백용)
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	// 플레이어가 아이템을 상점에 떨어뜨렸을 때 (판매 로직의 핵심!)
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	// 화면을 갱신하는 함수 (NPC의 현재 ShopInventory를 다시 그림)
	UFUNCTION()
	void RefreshShopUI();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UR1InventroySlotWidget> SlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UR1InventoryEntryWidget> EntryWidgetClass;

	UPROPERTY()
	TArray<TObjectPtr<UR1InventroySlotWidget>> SlotWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UR1InventoryEntryWidget>> EntryWidgets;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel_Slots;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel_Entries;

private:
	int32 X_COUNT = 10;
	int32 Y_COUNT = 5;

	UPROPERTY()
	TObjectPtr<AR1MerchantNPC> CurrentNPC;
};
