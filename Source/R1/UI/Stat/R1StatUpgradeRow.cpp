
#include "UI/Stat/R1StatUpgradeRow.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UR1StatUpgradeRow::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Upgrade)
	{
		Button_Upgrade->OnClicked.AddUniqueDynamic(this, &UR1StatUpgradeRow::Internal_OnButtonClicked);
	}
}

void UR1StatUpgradeRow::InjectData(const FText& InName, int32 InvestmentCount)
{
	if (Text_AttributeName)
	{
		Text_AttributeName->SetText(InName);
	}

	if (Text_StatValue)
	{
		Text_StatValue->SetText(FText::AsNumber(InvestmentCount));
	}
}

void UR1StatUpgradeRow::SetButtonEnabled(bool bEnabled)
{
	if (Button_Upgrade)
	{
		Button_Upgrade->SetIsEnabled(bEnabled);
	}
}

void UR1StatUpgradeRow::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (Text_AttributeName)
	{
		Text_AttributeName->SetText(EditorAttributeName);
	}
}

void UR1StatUpgradeRow::Internal_OnButtonClicked()
{
	if (OnUpgradeRowClicked.IsBound())
	{
		OnUpgradeRowClicked.Broadcast(StatTag);
	}
}