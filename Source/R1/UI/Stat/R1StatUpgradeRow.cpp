


#include "UI/Stat/R1StatUpgradeRow.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UR1StatUpgradeRow::InjectData(int32 InvestmentCount)
{
	if (Text_StatValue)
	{
		Text_StatValue->SetText(FText::AsNumber(InvestmentCount));
	}
}

FText UR1StatUpgradeRow::GetAttributeName() const
{
	return Text_AttributeName ? Text_AttributeName->GetText() : FText::GetEmpty();
}

