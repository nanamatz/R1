#include "UI/R1DamageTextWidget.h"
#include "Components/TextBlock.h"

void UR1DamageTextWidget::SetDamageInfo(const FR1DamageInfo& Info)
{
	if (Text_DamageAmount)
	{
		Text_DamageAmount->SetText(FText::AsNumber(Info.DamageAmount));

		if (Info.DamageType == ER1DamageType::Critical)
		{
			Text_DamageAmount->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		}
		else
		{
			Text_DamageAmount->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
	}

	OnSetDamageInfo(Info);
}

