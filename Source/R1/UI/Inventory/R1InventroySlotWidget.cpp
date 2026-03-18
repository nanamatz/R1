


#include "UI/Inventory/R1InventroySlotWidget.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"

UR1InventroySlotWidget::UR1InventroySlotWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UR1InventroySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SizeBox_Root->SetWidthOverride(50);
	SizeBox_Root->SetHeightOverride(50);
}

void UR1InventroySlotWidget::SetSlotState(ESlotHoverState InState)
{
	if (!Image_Background) return;

	switch (InState)
	{
	case ESlotHoverState::Normal:
		Image_Background->SetColorAndOpacity(NormalColor);
		break;
	case ESlotHoverState::Valid:
		Image_Background->SetColorAndOpacity(ValidColor);
		break;
	case ESlotHoverState::Invalid:
		Image_Background->SetColorAndOpacity(InvalidColor);
		break;
	}
}

