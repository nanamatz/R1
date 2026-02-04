


#include "UI/Inventory/R1InventroySlotWidget.h"
#include "Components/SizeBox.h"

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

