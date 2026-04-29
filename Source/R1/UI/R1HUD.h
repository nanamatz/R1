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

    void ShowMonsterInfo(class AR1Monster* Monster);
    void HideMonsterInfo();

    UUserWidget* GetInventoryWidget() const { return InventoryUIWidget; }
    UUserWidget* GetBaseUIWidget() const { return BaseUIWidget; }
    UUserWidget* GetGameOverUIWidget() const { return GameOverUIWidget; }
    UUserWidget* GetGameMenuUIWidget() const { return GameMenuUIWidget; }

    UPROPERTY(EditDefaultsOnly, Category = "UI|Audio")
    TObjectPtr<class UR1UISoundData> UISoundData;

protected:
    virtual void BeginPlay() override;

public:
	bool bIsInventoryUIVisible;
    bool bIsShopUIVisible;
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

public:
    // NPC가 호출할 상점 오픈 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void OpenShopUI(class AR1MerchantNPC* MerchantNPC);

    // 상점 닫기 함수 (UI의 닫기 버튼이나 ESC 키로 호출)
    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseShopUI();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> ShopWidgetClass;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UUserWidget> ShopSceneWidget = nullptr;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<class UR1ShopWidget> ShopWidget = nullptr;
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
    TSubclassOf<class UR1MonsterInfoSceneWidget> MonsterInfoWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> MiniMapUIWidgetClass;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<class UR1MonsterInfoSceneWidget> MonsterInfoWidget = nullptr;

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
