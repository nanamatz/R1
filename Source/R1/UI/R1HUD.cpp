#include "UI/R1HUD.h"
#include "Blueprint/UserWidget.h"

void AR1HUD::BeginPlay()
{
    Super::BeginPlay();

    // 안전 검사: 클래스와 플레이어 컨트롤러가 유효할 때만 생성
    APlayerController* PC = GetOwningPlayerController();
    if ( PC && !MyInventoryWidget)
    {
        MyInventoryWidget = CreateWidget<UUserWidget>(PC, InventoryWidgetClass);
        if (MyInventoryWidget)
        {
            MyInventoryWidget->AddToViewport();
            MyInventoryWidget->SetVisibility(ESlateVisibility::Hidden);
            bIsInventoryVisible = false;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create InventoryWidget from class %s"), *GetNameSafe(InventoryWidgetClass));
        }
    }
    if(PC && !BaseUIWidget)
    {
        BaseUIWidget = CreateWidget<UUserWidget>(PC, BaseUIWidgetClass);
        if (BaseUIWidget)
        {
            BaseUIWidget->AddToViewport();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create BaseUIWidget"));
        }
	}
}

void AR1HUD::ToggleInventory()
{
    if (!InventoryWidgetClass) return;

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
