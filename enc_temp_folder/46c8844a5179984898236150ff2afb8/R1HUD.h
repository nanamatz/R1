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
    void UpdateGameOverUI();

    UUserWidget* GetInventoryWidget() const { return MyInventoryWidget; }
    UUserWidget* GetBaseUIWidget() const { return BaseUIWidget; }
    UUserWidget* GetGameOverUIWidget() const { return GameOverUIWidget; }

public:
	bool bIsInventoryVisible;
    bool bIsGameOverUIVisible;

protected:
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UUserWidget> InventoryWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> BaseUIWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> GameOverUIWidgetClass;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UUserWidget> MyInventoryWidget = nullptr;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UUserWidget> BaseUIWidget = nullptr;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UUserWidget> GameOverUIWidget = nullptr;

    virtual void BeginPlay() override;
};
