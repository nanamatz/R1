

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1TitleScreenWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1TitleScreenWidget : public UR1UserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1TitleWidget> WBP_Title;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1MainMenuWidget> WBP_MainMenu;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1OptionsMenuWidget> WBP_OptionWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1MetaUpgradeWidget> WBP_UpgradeWidget;


	// UI 화면 전환 함수들
	void SwitchToTitle();
	void SwitchToMainMenu();
	void SwitchToOptions();
	void SwitchToMetaUpgrade();
};
