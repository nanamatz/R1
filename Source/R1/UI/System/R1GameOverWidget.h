

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
	// 위젯이 생성될 때 한 번 호출되는 초기화 함수 (BeginPlay와 비슷한 역할)
	virtual void NativeConstruct() override;

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
