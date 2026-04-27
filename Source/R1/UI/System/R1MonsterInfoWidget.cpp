
#include "UI/System/R1MonsterInfoWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UR1MonsterInfoWidget::UpdateMonsterInfo(const FString& Name, float HpRatio)
{
	if (HpBar)
	{
		HpBar->SetPercent(HpRatio);
	}

	if (MonsterName)
	{
		MonsterName->SetText(FText::FromString(Name));
	}
}
