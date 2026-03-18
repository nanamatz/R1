


#include "UI/Inventory/Item/R1ItemDragWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"

UR1ItemDragWidget::UR1ItemDragWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UR1ItemDragWidget::Init(const FVector2D& InWidgetSize, UTexture2D* InItemIcon, int32 InItemCount)
{
	if (!SizeBox_Root || !Image_Icon || !Text_Count)
	{
		UE_LOG(LogTemp, Error, TEXT("[UR1ItemDragWidget] 위젯 컴포넌트가 존재하지 않습니다! 블루프린트 설정이나 생성 시점을 확인하세요."));
		return;
	}

	SizeBox_Root->SetWidthOverride(InWidgetSize.X);
	SizeBox_Root->SetHeightOverride(InWidgetSize.Y);

	if (InItemIcon)
	{
		Image_Icon->SetBrushFromTexture(InItemIcon);
	}

	Text_Count->SetText((InItemCount >= 2) ? FText::AsNumber(InItemCount) : FText::GetEmpty());

}
