


#include "UI/Inventory/R1InventoryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI/R1HUD.h"
#include "Item/R1DragDropOperation.h"
#include "Item/R1InventorySubsystem.h"

void UR1InventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (UR1InventorySubsystem* Inventory = GetWorld()->GetSubsystem<UR1InventorySubsystem>())
	{
		// 🌟 1. 유저님이 이미 만들어두신 방송국(OnGoldChanged)에 수신기(UpdateGoldUI)를 연결!
		Inventory->OnGoldChanged.AddDynamic(this, &UR1InventoryWidget::UpdateGoldUI);

		// 🌟 2. 위젯이 처음 열릴 때 현재 골드량으로 초기 텍스트 세팅
		UpdateGoldUI(Inventory->GetGold());
	}

	if (Button_Close)
	{
		Button_Close->OnClicked.AddUniqueDynamic(this, &UR1InventoryWidget::OnCloseButtonClicked);
	}
}

bool UR1InventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	if (UR1DragDropOperation* DragDropOp = Cast<UR1DragDropOperation>(InOperation))
	{
		UR1InventorySubsystem* InvenSubsys = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
		if (InvenSubsys)
		{
			InvenSubsys->OnInventoryUpdated.Broadcast();
		}
	}

	return true;
}

void UR1InventoryWidget::UpdateGoldUI(int32 NewGold)
{
	if (Text_Gold)
	{
		Text_Gold->SetText(FText::AsNumber(NewGold));
	}
}

void UR1InventoryWidget::OnCloseButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	if (AR1HUD* HUD = Cast<AR1HUD>(PC->GetHUD()))
	{
		HUD->ToggleInventory();
	}
}