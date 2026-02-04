#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "R1UserWidget.h"
#include "R1HUD.generated.h"

/**
 * 
 */
UCLASS()
class R1_API AR1HUD : public AHUD
{
	GENERATED_BODY()

public:
    // 인벤토리 토글 함수
    void ToggleInventory();

private:
    UPROPERTY()
    TObjectPtr<UUserWidget> InventoryWidget;

public:
	bool bIsInventoryVisible;

protected:
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UUserWidget> InventoryWidgetClass;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UUserWidget> MyInventoryWidget = nullptr;

    virtual void BeginPlay() override;
};
