


#include "UI/Inventory/R1InventroySlotWidget.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Library/R1ItemFunctionLibrary.h"

UR1InventroySlotWidget::UR1InventroySlotWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UR1InventroySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SizeBox_Root->SetWidthOverride(50);
	SizeBox_Root->SetHeightOverride(50);

	RefreshColor();
}

void UR1InventroySlotWidget::SetSlotState(ESlotHoverState InState)
{
	CurrentHoverState = InState;
	RefreshColor();
}

void UR1InventroySlotWidget::SetItemRarity(bool bHasItem, EItemRarity InRarity)
{
	bIsOccupied = bHasItem;
	OccupiedRarity = InRarity;
	RefreshColor();
}

void UR1InventroySlotWidget::RefreshColor()
{
	if (!Image_Background || !Image_RarityBorder) return;

	// --- 1. 배경색 (Background) 설정 ---
	if (bIsOccupied)
	{
		FSlateColor RarityColor = UR1ItemFunctionLibrary::GetRarityColor(OccupiedRarity);
		// 배경은 희귀도 색상으로 칠하되, 아이콘이 잘 보이도록 약간 어둡게 처리할 수도 있습니다.
		Image_Background->SetColorAndOpacity(RarityColor.GetSpecifiedColor());
	}
	else
	{
		Image_Background->SetColorAndOpacity(NormalColor);
	}

	// --- 2. 테두리 (Border) 설정 ---
	// 기본적으로 테두리는 숨김
	Image_RarityBorder->SetVisibility(ESlateVisibility::Hidden);

	// 드래그 중이라면 테두리에 초록/빨강 표시 (배경색은 유지되어 아이템 위치 파악 용이)
	if (CurrentHoverState == ESlotHoverState::Valid)
	{
		Image_RarityBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Image_RarityBorder->SetColorAndOpacity(ValidColor);
	}
	else if (CurrentHoverState == ESlotHoverState::Invalid)
	{
		Image_RarityBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Image_RarityBorder->SetColorAndOpacity(InvalidColor);
	}
	// 드래그 중이 아닐 때 아이템이 있다면, 테두리를 아주 밝게 칠해 등급을 강조
	else if (bIsOccupied)
	{
		FSlateColor RarityColor = UR1ItemFunctionLibrary::GetRarityColor(OccupiedRarity);
		Image_RarityBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		
		// 테두리는 배경보다 더 밝게(Vivid) 보이도록 설정
		FLinearColor BrightColor = RarityColor.GetSpecifiedColor();
		BrightColor.A = 1.0f; // 불투명도 강조
		Image_RarityBorder->SetColorAndOpacity(BrightColor);
	}
}

