

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1GameMenuWIdget.generated.h"

class UR1CommonButton;

/**
 * 
 */
UCLASS()
class R1_API UR1GameMenuWIdget : public UR1UserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void RefreshLocalization();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_Paused;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1CommonButton> Button_Resume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1CommonButton> Button_Options;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1CommonButton> Button_Exit;

	UFUNCTION()
	void OnResumeButtonClicked();

	UFUNCTION()
	void OnOptionsButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();


	UPROPERTY(EditDefaultsOnly, Category = "Level")
	FName TitleLevelName = TEXT("DefaultMap");

protected:
	FTimerHandle TransitionTimerHandle;
	void ExecuteExit();
};
