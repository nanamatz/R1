


#include "UI/Inventory/Item/R1StatRowWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UR1StatRowWidget::InitStatRow(UTexture2D* Icon, const FString& StatText, const FString& StatValue)
{
	if (Image_StatIcon)
	{
		if (Icon)
		{
			Image_StatIcon->SetBrushFromTexture(Icon);
			Image_StatIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			// 아이콘이 없으면 이미지 영역을 압축해 버립니다.
			Image_StatIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (Text_StatInfo)
	{
		Text_StatInfo->SetText(FText::FromString(StatText));
	}

	if (Text_StatValue)
	{
		if (!StatValue.IsEmpty())
		{
			Text_StatValue->SetText(FText::FromString(StatValue));
			Text_StatValue->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Text_StatValue->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
