


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
		// 대사 배열에 데이터가 1개 이상 있는지 안전 검사
		if (NPCData->GreetingDialogues.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, NPCData->GreetingDialogues.Num() - 1);
			FText SelectedText = NPCData->GreetingDialogues[RandomIndex];

			// 뽑힌 대사를 UI에 적용
			Text_Greeting->SetText(SelectedText);

			if (!SelectedText.IsEmptyOrWhitespace())
			{
				Text_Greeting->SetText(SelectedText);
			}
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