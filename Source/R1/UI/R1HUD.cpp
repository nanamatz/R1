#include "UI/R1HUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/System/R1FloorGuideSceneWidget.h"
#include "Map/R1MapGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "System/R1LoadingSubSystem.h"
#include "Item/R1InventorySubsystem.h"
#include "Object/R1MerchantNPC.h"
#include "UI/Shop/R1ShopWidget.h"

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
        if (!MiniMapUIWidget)
        {
            MiniMapUIWidget = CreateWidget<UUserWidget>(PC, MiniMapUIWidgetClass);
            if (MiniMapUIWidget)
            {
                MiniMapUIWidget->AddToViewport(5);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to create GameMenuUIWidget"));
            }
        }
        if (!FloorGuideSceneWidget && FloorGuideSceneWidgetClass)
        {
            FloorGuideSceneWidget = CreateWidget<UR1FloorGuideSceneWidget>(PC, FloorGuideSceneWidgetClass);
            if (FloorGuideSceneWidget)
            {
                FloorGuideSceneWidget->AddToViewport(20);
                FloorGuideSceneWidget->SetVisibility(ESlateVisibility::Hidden);
            }
        }
        if (AR1MapGenerator* MapGen = Cast<AR1MapGenerator>(UGameplayStatics::GetActorOfClass(this, AR1MapGenerator::StaticClass())))
        {
            MapGen->OnMapGenerated.AddDynamic(this, &AR1HUD::HandleMapGenerated);
        }
        if (UR1LoadingSubSystem* LoadingSubsystem = GetGameInstance()->GetSubsystem<UR1LoadingSubSystem>())
        {
            LoadingSubsystem->OnLoadingScreenHidden.AddDynamic(this, &AR1HUD::HandleLoadingScreenHidden);
        }
    }

}

void AR1HUD::HandleMapGenerated(const TArray<struct FR1MapNode>& MapData)
{
    if (!FloorGuideSceneWidget) return;

    AR1MapGenerator* MapGen = Cast<AR1MapGenerator>(UGameplayStatics::GetActorOfClass(this, AR1MapGenerator::StaticClass()));
    if (MapGen)
    {
        PendingFloorLevel = static_cast<ER1FloorLevel>(MapGen->CurrentFloorIndex);

        bIsFloorGuidePending = true;
    }
}

void AR1HUD::HandleLoadingScreenHidden()
{
    if (bIsFloorGuidePending && FloorGuideSceneWidget)
    {
        // 드디어 화면에 애니메이션을 띄웁니다!
        FloorGuideSceneWidget->ShowFloorGuide(PendingFloorLevel);

        bIsFloorGuidePending = false;
    }
}

void AR1HUD::OpenShopUI(AR1MerchantNPC* MerchantNPC)
{
    if (!MerchantNPC || !ShopWidgetClass) return;

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    // 1. 위젯이 없다면 생성
    if (!ShopSceneWidget)
    {
        ShopSceneWidget = CreateWidget<UUserWidget>(PC, ShopWidgetClass);
    }

    if(!ShopWidget)
    {
        ShopWidget = Cast<UR1ShopWidget>(ShopSceneWidget);
	}

    // 2. 위젯 띄우기 및 데이터 전달
    if (ShopWidget && !ShopWidget->IsInViewport())
    {
        // 우리가 열심히 만든 슬롯 그리드 갱신 함수를 여기서 호출!
        // (ShopWidget 내부에서 자신이 가지고 있는 ShopSlotsWidget의 InitShopGrid를 호출하도록 연결해야 합니다)
        ShopWidget->InitShop(MerchantNPC);

        ShopWidget->AddToViewport();

        // 서브시스템 상태 업데이트
        if (UR1InventorySubsystem* InvenSubsys = GetWorld()->GetSubsystem<UR1InventorySubsystem>())
        {
            InvenSubsys->bIsShopOpen = true;
        }

        // 인풋 모드 변경 (UI 중심)
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(ShopWidget->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
}

void AR1HUD::CloseShopUI()
{
    if (ShopWidget && ShopWidget->IsInViewport())
    {
        ShopWidget->RemoveFromParent();

        if (UR1InventorySubsystem* InvenSubsys = GetWorld()->GetSubsystem<UR1InventorySubsystem>())
        {
            InvenSubsys->bIsShopOpen = false;
        }

        APlayerController* PC = GetOwningPlayerController();
        if (PC)
        {
            // 게임 전용 모드로 복구
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = false;
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


