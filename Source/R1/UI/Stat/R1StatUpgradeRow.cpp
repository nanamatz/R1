


#include "UI/Stat/R1StatUpgradeRow.h"
#include "Components/TextBlock.h"

void UR1StatUpgradeRow::InjectData(int32 InvestmentCount)
{
	if (Text_StatValue)
	{
		Text_StatValue->SetText(FText::AsNumber(InvestmentCount));
	}
}

