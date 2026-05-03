#include "UI/R1DamageTextWidget.h"
#include "Components/TextBlock.h"

void UR1DamageTextWidget::SetDamageInfo(const FR1DamageInfo& Info)
{
	if (Text_DamageAmount)
	{
		Text_DamageAmount->SetText(FText::AsNumber(FMath::RoundToInt(Info.DamageAmount)));
	}

	OnSetDamageInfo(Info);
}

void UR1DamageTextWidget::HandleAnimationFinished()
{
	OnAnimationFinished.Broadcast();
}
