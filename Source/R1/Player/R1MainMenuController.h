

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "R1Define.h"
#include "R1MainMenuController.generated.h"

class UUserWidget;
class UR1TitleWidget;
class UR1MainMenuWidget;
class UR1OptionsWidget;
/**
 * 
 */
UCLASS()
class R1_API AR1MainMenuController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UR1TitleScreenWidget> TitleScreenWidgetClass;

	UPROPERTY()
	TObjectPtr<class UR1TitleScreenWidget> TitleScreenWidget;

	// 레벨에 배치된 카메라들을 담을 포인터
	TObjectPtr<AActor> TitleCamera;
	TObjectPtr<AActor> OptionsCamera;
	TObjectPtr<AActor> MainMenuCamera;
	TObjectPtr<AActor> GameStartCamera;

protected:
	// 현재 화면 상태를 기억할 변수
	EMenuState CurrentMenuState;

protected:
	// 키 입력 바인딩을 위한 엔진 기본 함수
	virtual void SetupInputComponent() override;

	// 아무 키나 눌렸을 때 실행될 함수
	void OnGlobalInputPressed();
public:
	// ESC나 뒤로 가기 버튼을 눌렀을 때 호출될 만능 함수!
	UFUNCTION(BlueprintCallable)
	void GoBack();

public:
	UFUNCTION(BlueprintCallable) 
	void ShowTitleScreen();

	UFUNCTION(BlueprintCallable) 
	void ShowMainMenuScreen();

	UFUNCTION(BlueprintCallable) 
	void ShowOptionsScreen();

	UFUNCTION(BlueprintCallable)
	void ShowGameStartCamera();

private:
	void OnMainMenuCameraBlendFinished();

	FTimerHandle MenuTimerHandle;
};
