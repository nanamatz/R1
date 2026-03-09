

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1MainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1MainMenuWidget : public UR1UserWidget
{
	GENERATED_BODY()
protected:
	// 위젯이 생성될 때 한 번 호출되는 초기화 함수 (BeginPlay와 비슷한 역할)
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Button_NewRun;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Button_Continue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Button_Options;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Button_Exit;


	UFUNCTION()
	void OnNewRunButtonClicked();

	UFUNCTION()
	void OnContinueButtonClicked();

	UFUNCTION()
	void OnOptionsButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

	UPROPERTY(EditDefaultsOnly, Category = "Level")
	FName GameLevelName = TEXT("MainMap"); // 본인의 실제 1층 맵 이름으로 변경하세요!
};
