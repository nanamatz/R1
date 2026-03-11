


#include "UI/System/R1TitleScreenWidget.h"
#include "UI/System/R1TitleWidget.h"
#include "UI/System/R1MainMenuWidget.h"

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
}

void UR1TitleScreenWidget::SwitchToMainMenu()
{
	if (WBP_Title)
	{
		WBP_Title->SetVisibility(ESlateVisibility::Hidden);
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
