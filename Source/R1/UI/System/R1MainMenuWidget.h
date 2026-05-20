

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1MainMenuWidget.generated.h"

class UR1CommonButton;

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

	// 🌟 키보드 입력을 감지하는 함수 오버라이드
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1CommonButton> Button_NewRun;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1CommonButton> Button_Continue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1CommonButton> Button_Upgrade;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1CommonButton> Button_Options;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1CommonButton> Button_Exit;

	UFUNCTION()
	void OnUpgradeButtonClicked();

	UFUNCTION()
	void OnNewRunButtonClicked();

	UFUNCTION()
	void OnContinueButtonClicked();

	UFUNCTION()
	void OnOptionsButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

	UPROPERTY(EditDefaultsOnly, Category = "Level")
	FName GameLevelName = TEXT("MainMap"); 

protected:
	// 타이머 핸들
	FTimerHandle LevelLoadTimerHandle;

	// 새 게임인지 이어하기인지 구분하는 플래그
	bool bIsNewRunPending = false;

	// 버튼 클릭 시 연출을 시작하는 공통 함수
	void StartGameTransition(bool bIsNewRun);

	// 연출이 끝난 후 실제로 맵을 여는 함수
	void ExecuteLevelLoad();
};
