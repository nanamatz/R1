


#include "UI/System/R1TitleScreenWidget.h"
#include "UI/System/R1TitleWidget.h"
#include "UI/System/R1MainMenuWidget.h"
#include "UI/Progression/R1MetaUpgradeWidget.h"

void UR1TitleScreenWidget::SwitchToTitle()
{
	if (WBP_Title)
	{
		WBP_Title->SetVisibility(ESlateVisibility::Visible);
		WBP_Title->SetFocus(); // 키보드 포커스 주기
	}
	if (WBP_MainMenu)
	{
		WBP_MainMenu->SetVisibility(ESlateVisibility::Hidden);
	}
	if (WBP_UpgradeWidget)
	{
		WBP_UpgradeWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UR1TitleScreenWidget::SwitchToMainMenu()
{
	if (WBP_Title)
	{
		WBP_Title->SetVisibility(ESlateVisibility::Hidden);
	}
	if (WBP_UpgradeWidget)
	{
		WBP_UpgradeWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	if (WBP_MainMenu)
	{
		WBP_MainMenu->SetVisibility(ESlateVisibility::Visible);
		WBP_MainMenu->SetFocus();
	}
}

void UR1TitleScreenWidget::SwitchToOptions()
{

}

void UR1TitleScreenWidget::SwitchToMetaUpgrade()
{
	if (WBP_Title)
	{
		WBP_Title->SetVisibility(ESlateVisibility::Hidden);
	}
	if (WBP_UpgradeWidget)
	{
		WBP_UpgradeWidget->SetVisibility(ESlateVisibility::Visible);
		WBP_UpgradeWidget->RefreshUI();
		WBP_UpgradeWidget->SetFocus();
	}
	if (WBP_MainMenu)
	{
		WBP_MainMenu->SetVisibility(ESlateVisibility::Hidden);
	}
}
