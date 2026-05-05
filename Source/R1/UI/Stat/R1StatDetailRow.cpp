


#include "UI/Stat/R1StatDetailRow.h"
#include "Components/TextBlock.h"

void UR1StatDetailRow::InjectData(const FText& FormattedValue)
{
	if (Text_Amount)
	{
		Text_Amount->SetText(FormattedValue);
	}
}

