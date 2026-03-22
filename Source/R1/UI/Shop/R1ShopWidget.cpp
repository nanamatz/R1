


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

	// 자식 위젯인 6x3 그리드에게 NPC 데이터를 넘겨서 아이템을 그리게 합니다.
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

void UR1ShopWidget::OnCloseButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	// HUD를 찾아 상점 닫기 로직을 실행합니다.
	if (AR1HUD* HUD = Cast<AR1HUD>(PC->GetHUD()))
	{
		HUD->CloseShopUI();
	}
}