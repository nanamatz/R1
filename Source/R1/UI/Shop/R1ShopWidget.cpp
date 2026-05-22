


#include "UI/Shop/R1ShopWidget.h"
#include "UI/Shop/R1ShopSlotsWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Object/R1MerchantNPC.h"
#include "UI/R1HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Data/R1ShopNPCData.h"
#include "System/R1LocalizationSubsystem.h"

UR1ShopWidget::UR1ShopWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UR1ShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Close)
	{
		Button_Close->OnClicked.AddUniqueDynamic(this, &UR1ShopWidget::OnCloseButtonClicked);
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.AddUObject(this, &UR1ShopWidget::RefreshLocalization);
		}
	}
}

void UR1ShopWidget::NativeDestruct()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.RemoveAll(this);
		}
	}
	Super::NativeDestruct();
}

void UR1ShopWidget::InitShop(AR1MerchantNPC* InNPC)
{
	if (!InNPC || !ShopSlotsWidget) return;

	ShopSlotsWidget->InitShopGrid(InNPC);

	// 새 NPC를 만날 때마다 인사말 재추첨
	CachedGreetingIndex = -1;

	if (InNPC->CurrentNPCData)
	{
		InitShopNPC(InNPC->CurrentNPCData);
	}
}

void UR1ShopWidget::InitShopNPC(UR1ShopNPCData* NPCData)
{
	if (!NPCData) return;

	CachedNPCData = NPCData;

	UGameInstance* GI = GetGameInstance();
	UR1LocalizationSubsystem* LocSub = GI ? GI->GetSubsystem<UR1LocalizationSubsystem>() : nullptr;

	// 1. 이름 세팅
	if (Text_ShopNPC)
	{
		FText Name = (LocSub && !NPCData->NPCNameKey.IsNone())
			? LocSub->GetText(NPCData->NPCNameKey)
			: NPCData->NPCName;
		Text_ShopNPC->SetText(Name);
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
			Image_NPC_Portrait->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 3. 인사말 세팅
	if (Text_Greeting)
	{
		// 아직 추첨하지 않았다면 랜덤 인덱스 선택
		if (CachedGreetingIndex < 0)
		{
			int32 MaxIndex = FMath::Max(NPCData->GreetingKeys.Num(), NPCData->GreetingDialogues.Num()) - 1;
			if (MaxIndex >= 0)
			{
				CachedGreetingIndex = FMath::RandRange(0, MaxIndex);
			}
		}

		FText Greeting = FText::GetEmpty();
		if (CachedGreetingIndex >= 0)
		{
			if (LocSub && NPCData->GreetingKeys.IsValidIndex(CachedGreetingIndex) && !NPCData->GreetingKeys[CachedGreetingIndex].IsNone())
			{
				Greeting = LocSub->GetText(NPCData->GreetingKeys[CachedGreetingIndex]);
			}
			else if (NPCData->GreetingDialogues.IsValidIndex(CachedGreetingIndex))
			{
				Greeting = NPCData->GreetingDialogues[CachedGreetingIndex];
			}
		}

		if (!Greeting.IsEmpty())
		{
			Text_Greeting->SetText(Greeting);
			Text_Greeting->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Text_Greeting->SetText(FText::GetEmpty());
			Text_Greeting->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UR1ShopWidget::RefreshLocalization()
{
	if (CachedNPCData)
	{
		InitShopNPC(CachedNPCData);
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
