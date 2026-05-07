


#include "UI/Shop/R1ShopWidget.h"
#include "UI/Shop/R1ShopSlotsWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Object/R1MerchantNPC.h"
#include "UI/R1HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Data/R1ShopNPCData.h"

UR1ShopWidget::UR1ShopWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UR1ShopWidget::InitShop(AR1MerchantNPC* InNPC)
{
	if (!InNPC || !ShopSlotsWidget) return;

	ShopSlotsWidget->InitShopGrid(InNPC);

	// 새로운 NPC를 만날 때만 대사를 새로 뽑을 수 있도록 캐시를 초기화할 수도 있지만,
	// 여기서는 InitShopNPC 내부에서 판단하도록 넘깁니다.
	CachedGreeting = FText::GetEmpty();

	if (InNPC->CurrentNPCData)
	{
		InitShopNPC(InNPC->CurrentNPCData);
	}
}

void UR1ShopWidget::InitShopNPC(UR1ShopNPCData* NPCData)
{
	if (!NPCData) return;

	// 1. 이름 세팅
	if (Text_ShopNPC)
	{
		Text_ShopNPC->SetText(NPCData->NPCName);
		Text_ShopNPC->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	// 2. 초상화 세팅
	if (Image_NPC_Portrait)
	{
		if (NPCData->NPCPortrait)
		{
			Image_NPC_Portrait->SetBrushFromTexture(NPCData->NPCPortrait);
			Image_NPC_Portrait->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			// 이미지가 없는 NPC라면 가려줍니다.
			Image_NPC_Portrait->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (Text_Greeting)
	{
		// 이미 대사가 캐싱되어 있다면 (구매 시 리프레시 등) 새로 뽑지 않고 그대로 씁니다.
		if (CachedGreeting.IsEmpty())
		{
			if (NPCData->GreetingDialogues.Num() > 0)
			{
				int32 RandomIndex = FMath::RandRange(0, NPCData->GreetingDialogues.Num() - 1);
				CachedGreeting = NPCData->GreetingDialogues[RandomIndex];
			}
		}

		if (!CachedGreeting.IsEmpty())
		{
			Text_Greeting->SetText(CachedGreeting);
			Text_Greeting->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Text_Greeting->SetText(FText::GetEmpty());
			Text_Greeting->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
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