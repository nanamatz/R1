#include "UI/Shop/R1ShopWidget.h"
#include "UI/Shop/R1ShopGridWidget.h"
#include "Item/R1InventorySubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UR1ShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWorld* World = GetWorld())
	{
		if (UR1InventorySubsystem* InventorySubsystem = World->GetSubsystem<UR1InventorySubsystem>())
		{
			// 초기 골드 표시
			UpdateGoldDisplay(InventorySubsystem->GetGold());

			// 골드 변경 시 콜백 등록
			InventorySubsystem->OnGoldChanged.AddDynamic(this, &UR1ShopWidget::UpdateGoldDisplay);
		}
	}

	if (Button_Close)
	{
		Button_Close->OnClicked.AddDynamic(this, &UR1ShopWidget::OnCloseButtonClicked);
	}
}

void UR1ShopWidget::SetShopItems(const TArray<UR1ItemInstance*>& Items)
{
	if (ShopGrid)
	{
		ShopGrid->InitShopGrid(Items);
	}
}

void UR1ShopWidget::UpdateGoldDisplay(int32 NewGold)
{
	if (Text_CurrentGold)
	{
		Text_CurrentGold->SetText(FText::AsNumber(NewGold));
	}
}

void UR1ShopWidget::OnCloseButtonClicked()
{
	if (UWorld* World = GetWorld())
	{
		if (UR1InventorySubsystem* InventorySubsystem = World->GetSubsystem<UR1InventorySubsystem>())
		{
			InventorySubsystem->bIsShopOpen = false;
		}
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	RemoveFromParent();
}
