


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

FReply UR1ShopWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	// 🌟 좌클릭이나 우클릭 시 여기서 이벤트를 '소모(Handled)'하여 
	// 마우스 클릭이 게임 월드(PlayerController)로 넘어가는 것을 완벽히 막습니다!
	return FReply::Handled();
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