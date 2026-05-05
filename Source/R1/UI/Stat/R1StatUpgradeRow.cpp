


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

void UR1StatUpgradeRow::Internal_OnButtonClicked()
{
	if (OnUpgradeRowClicked.IsBound())
	{
		OnUpgradeRowClicked.Broadcast(StatTag);
	}
}

