

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1GameMenuWIdget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1GameMenuWIdget : public UR1UserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Button_Resume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Button_Options;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Button_Exit;

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
