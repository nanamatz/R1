


#include "UI/Stat/R1StatDetailRow.h"
#include "Components/TextBlock.h"

void UR1StatDetailRow::InjectData(const FText& FormattedValue)
{
	if (Text_Amount)
	{
		Text_Amount->SetText(FormattedValue);
	}
}

FText UR1StatDetailRow::GetAttributeName() const
{
	return Text_AttributeName ? Text_AttributeName->GetText() : FText::GetEmpty();
}

