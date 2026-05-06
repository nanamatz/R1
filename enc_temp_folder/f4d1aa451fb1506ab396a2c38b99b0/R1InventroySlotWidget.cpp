


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

	// 기본 상태: 배경은 NormalColor(보통 어두운 색), 테두리는 숨김
	Image_Background->SetColorAndOpacity(NormalColor);
	Image_RarityBorder->SetVisibility(ESlateVisibility::Hidden);

	// 1순위: 플레이어가 드래그 중이라면 테두리에 유효/무효 색상 표시
	if (CurrentHoverState == ESlotHoverState::Valid)
	{
		Image_RarityBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Image_RarityBorder->SetColorAndOpacity(ValidColor);
		return;
	}
	else if (CurrentHoverState == ESlotHoverState::Invalid)
	{
		Image_RarityBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Image_RarityBorder->SetColorAndOpacity(InvalidColor);
		return;
	}

	// 2순위: 아이템이 들어있다면 테두리에 희귀도 색상 적용
	if (bIsOccupied)
	{
		FSlateColor RarityColor = UR1ItemFunctionLibrary::GetRarityColor(OccupiedRarity);
		Image_RarityBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Image_RarityBorder->SetColorAndOpacity(RarityColor.GetSpecifiedColor());
	}
}

