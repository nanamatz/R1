#include "UI/R1HUD.h"
#include "Blueprint/UserWidget.h"

void AR1HUD::BeginPlay()
{
    Super::BeginPlay();

    // 안전 검사: 클래스와 플레이어 컨트롤러가 유효할 때만 생성
    APlayerController* PC = GetOwningPlayerController();
    if (PC)
    {
        if (!InventoryUIWidget)
        {
            InventoryUIWidget = CreateWidget<UUserWidget>(PC, InventoryWidgetClass);
            if (InventoryUIWidget)
            {
                InventoryUIWidget->AddToViewport(10); // 인벤토리가 다른 UI보다 위에 오도록 설정
                InventoryUIWidget->SetVisibility(ESlateVisibility::Hidden);
                bIsInventoryUIVisible = false;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to create InventoryWidget from class %s"), *GetNameSafe(InventoryWidgetClass));
            }
        }
        if (!BaseUIWidget)
        {
            BaseUIWidget = CreateWidget<UUserWidget>(PC, BaseUIWidgetClass);
            if (BaseUIWidget)
            {
                BaseUIWidget->AddToViewport(5);
                //BaseUIWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible); // 전체 화면 UI가 클릭을 막지 않도록 설정
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to create BaseUIWidget"));
            }
        }
        if (!GameOverUIWidget)
        {
            GameOverUIWidget = CreateWidget<UUserWidget>(PC, GameOverUIWidgetClass);
            if (GameOverUIWidget)
            {
                GameOverUIWidget->AddToViewport(15);
                GameOverUIWidget->SetVisibility(ESlateVisibility::Hidden);
                bIsGameOverUIVisible = false;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to create GameOverUIWidget"));
            }
        }
        if (!GameMenuUIWidget)
        {
            GameMenuUIWidget = CreateWidget<UUserWidget>(PC, GameMenuUIWidgetClass);
            if (GameMenuUIWidget)
            {
                GameMenuUIWidget->AddToViewport(15);
                GameMenuUIWidget->SetVisibility(ESlateVisibility::Hidden);
                bIsGameMenuUIVisible = false;

            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to create GameMenuUIWidget"));
            }
        }
    }

}

void AR1HUD::ToggleInventory()
{
    if (!InventoryWidgetClass || !InventoryUIWidget) return;

    if (bIsInventoryUIVisible)
    {
        InventoryUIWidget->SetVisibility(ESlateVisibility::Hidden);
		bIsInventoryUIVisible = false;
    }
    else
    {
        InventoryUIWidget->SetVisibility(ESlateVisibility::Visible);
		bIsInventoryUIVisible = true;
    }
}

void AR1HUD::UpdateGameOverUI()
{
    if (!GameOverUIWidgetClass || !GameOverUIWidget) return;

    if (bIsGameOverUIVisible)
    {
        GameOverUIWidget->SetVisibility(ESlateVisibility::Hidden);
        bIsGameOverUIVisible = false;
    }
    else
    {
        GameOverUIWidget->SetVisibility(ESlateVisibility::Visible);
        bIsGameOverUIVisible = true;
    }
}

void AR1HUD::ToggleGameMenu()
{
    if (!GameMenuUIWidgetClass || !GameMenuUIWidget) return;

    if (bIsGameMenuUIVisible)
    {
        GameMenuUIWidget->SetVisibility(ESlateVisibility::Hidden);
        bIsGameMenuUIVisible = false;
    }
    else
    {
        GameMenuUIWidget->SetVisibility(ESlateVisibility::Visible);
        bIsGameMenuUIVisible = true;
    }
}


