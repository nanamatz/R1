#include "UI/Stat/R1StatDetailRow.h"
#include "Components/TextBlock.h"

void UR1StatDetailRow::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (Text_AttributeName)
	{
		Text_AttributeName->SetText(EditorAttributeName);
	}
}

void UR1StatDetailRow::InjectData(const FText& InName, const FText& InValue)
{
	if (Text_AttributeName)
	{
		Text_AttributeName->SetText(InName);
	}

	if (Text_Amount)
	{
		Text_Amount->SetText(InValue);
	}
}

