

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1GameOverWidget.generated.h"

class UR1CommonButton;

/**
 * 
 */
UCLASS()
class R1_API UR1GameOverWidget : public UR1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void RefreshLocalization();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_YouDied;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1CommonButton> Button_Retry;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1CommonButton> Button_Exit;

private:
	UFUNCTION()
	void OnRetryClicked();

	UFUNCTION()
	void OnExitClicked();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Level")
	FName TitleLevelName = TEXT("DefaultMap");

	FTimerHandle TransitionTimerHandle;

	void ExecuteRestart();
	void ExecuteExit();
};
