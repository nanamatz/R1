#include "UI/R1HUD.h"
#include "Blueprint/UserWidget.h"

void AR1HUD::BeginPlay()
{
    Super::BeginPlay();

    // 안전 검사: 클래스와 플레이어 컨트롤러가 유효할 때만 생성
    APlayerController* PC = GetOwningPlayerController();
    if (PC)
    {
        if (!MyInventoryWidget)
        {
            MyInventoryWidget = CreateWidget<UUserWidget>(PC, InventoryWidgetClass);
            if (MyInventoryWidget)
            {
                MyInventoryWidget->AddToViewport(10); // 인벤토리가 다른 UI보다 위에 오도록 설정
                MyInventoryWidget->SetVisibility(ESlateVisibility::Hidden);
                bIsInventoryVisible = false;
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
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to create GameOverUIWidget"));
            }
        }
    }

}

void AR1HUD::ToggleInventory()
{
    if (!InventoryWidgetClass || !MyInventoryWidget) return;

    if (bIsInventoryVisible)
    {
        MyInventoryWidget->SetVisibility(ESlateVisibility::Hidden);
		bIsInventoryVisible = false;
    }
    else
    {
        MyInventoryWidget->SetVisibility(ESlateVisibility::Visible);
		bIsInventoryVisible = true;
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


