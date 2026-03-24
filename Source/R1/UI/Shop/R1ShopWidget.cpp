


#include "UI/Shop/R1ShopWidget.h"
#include "UI/Shop/R1ShopSlotsWidget.h"
#include "Components/Button.h"
#include "Object/R1MerchantNPC.h"
#include "UI/R1HUD.h"
#include "Kismet/GameplayStatics.h"

UR1ShopWidget::UR1ShopWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UR1ShopWidget::InitShop(AR1MerchantNPC* InNPC)
{
	if (!InNPC || !ShopSlotsWidget) return;

	ShopSlotsWidget->InitShopGrid(InNPC);
}

void UR1ShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Close)
	{
		Button_Close->OnClicked.AddUniqueDynamic(this, &UR1ShopWidget::OnCloseButtonClicked);
	}
}

FReply UR1ShopWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	return FReply::Handled();
}

void UR1ShopWidget::OnCloseButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	if (AR1HUD* HUD = Cast<AR1HUD>(PC->GetHUD()))
	{
		HUD->CloseShopUI();
	}
}