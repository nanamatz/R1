

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "R1InventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1InventoryWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_Gold;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Button_Close;

	UFUNCTION()
	void OnCloseButtonClicked();
	UFUNCTION()
	void UpdateGoldUI(int32 NewGold);
};
