


#include "Item/R1ItemDragWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"

UR1ItemDragWidget::UR1ItemDragWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UR1ItemDragWidget::Init(const FVector2D& InWidgetSize, UTexture2D* InItemIcon, int32 InItemCount)
{
	SizeBox_Root->SetWidthOverride(InWidgetSize.X);
	SizeBox_Root->SetHeightOverride(InWidgetSize.Y);

	Image_Icon->SetBrushFromTexture(InItemIcon);
	Text_Count->SetText((InItemCount >= 2) ? FText::AsNumber(InItemCount) : FText::GetEmpty());

}
