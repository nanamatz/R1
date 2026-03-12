#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "R1Define.h"
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
    void ToggleGameMenu();

    UUserWidget* GetInventoryWidget() const { return InventoryUIWidget; }
    UUserWidget* GetBaseUIWidget() const { return BaseUIWidget; }
    UUserWidget* GetGameOverUIWidget() const { return GameOverUIWidget; }
    UUserWidget* GetGameMenuUIWidget() const { return GameMenuUIWidget; }

protected:
    virtual void BeginPlay() override;

public:
	bool bIsInventoryUIVisible;
    bool bIsGameOverUIVisible;
    bool bIsGameMenuUIVisible;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UR1FloorGuideSceneWidget> FloorGuideSceneWidgetClass;

    UPROPERTY()
    TObjectPtr<class UR1FloorGuideSceneWidget> FloorGuideSceneWidget;

    UFUNCTION()
    void HandleMapGenerated(const TArray<struct FR1MapNode>& MapData);

    UFUNCTION()
    void HandleLoadingScreenHidden();
protected:
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UUserWidget> InventoryWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> BaseUIWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> GameOverUIWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> GameMenuUIWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> MiniMapUIWidgetClass;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UUserWidget> MiniMapUIWidget = nullptr;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UUserWidget> InventoryUIWidget = nullptr;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UUserWidget> BaseUIWidget = nullptr;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UUserWidget> GameOverUIWidget = nullptr;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UUserWidget> GameMenuUIWidget = nullptr;

private:
    bool bIsFloorGuidePending = false;
    ER1FloorLevel PendingFloorLevel;

};
