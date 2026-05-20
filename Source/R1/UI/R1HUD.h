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
    void ToggleCharacterStatUI();
    void UpdateGameOverUI();
    void ToggleGameMenu();
    void ToggleOptionsUI();

    void ShowMonsterInfo(class AR1Monster* Monster);
    void HideMonsterInfo();
    void ShowBossInfo(class AR1Boss* InBoss);
    void HideBossInfo();

    // 옵션 메뉴 취소(ESC) 요청
    void CloseOptionsUIWithCancel();

    //UUserWidget* GetInventoryWidget() const { return InventoryUIWidget; }
    //UUserWidget* GetBaseUIWidget() const { return BaseUIWidget; }
    //UUserWidget* GetGameOverUIWidget() const { return GameOverUIWidget; }
    //UUserWidget* GetGameMenuUIWidget() const { return GameMenuUIWidget; }

    UPROPERTY(EditDefaultsOnly, Category = "UI|Audio")
    TObjectPtr<class UR1UISoundData> UISoundData;

protected:
    virtual void BeginPlay() override;

public:
	bool bIsInventoryUIVisible;
    bool bIsShopUIVisible;
    bool bIsGameOverUIVisible;
    bool bIsGameMenuUIVisible;
    bool bIsCharacterStatUIVisible;
    bool bIsOptionsUIVisible;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UR1FloorGuideSceneWidget> FloorGuideSceneWidgetClass;

    UPROPERTY()
    TObjectPtr<class UR1FloorGuideSceneWidget> FloorGuideSceneWidget;

    UFUNCTION()
    void HandleMapGenerated(const TArray<struct FR1MapNode>& MapData);

    UFUNCTION()
    void HandleLoadingScreenHidden();

    UFUNCTION()
    void SyncGameplaySettings();

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

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UUserWidget> CharacterStatUISceneWidget = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> CharacterStatUIWidgetClass;


    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UUserWidget> InventoryWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> BaseUIWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> GameOverUIWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> GameMenuUIWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> OptionsWidgetClass;

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

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<class UR1GameOptionsMenuSceneWidget> GameOptionsSceneWidget = nullptr;

private:
    bool bIsFloorGuidePending = false;
    ER1FloorLevel PendingFloorLevel;

    TWeakObjectPtr<class AR1Boss> CurrentBoss;
};
