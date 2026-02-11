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

    UUserWidget* GetInventoryWidget() const { return MyInventoryWidget; }
    UUserWidget* GetBaseUIWidget() const { return BaseUIWidget; }

public:
	bool bIsInventoryVisible;

protected:
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UUserWidget> InventoryWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> BaseUIWidgetClass;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UUserWidget> MyInventoryWidget = nullptr;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UUserWidget> BaseUIWidget = nullptr;

    virtual void BeginPlay() override;
};
