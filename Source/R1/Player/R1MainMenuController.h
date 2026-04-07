

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
	TObjectPtr<AActor> MetaUpgradeCamera;

protected:
	// 현재 화면 상태를 기억할 변수
	EMenuState CurrentMenuState;

protected:
	// 키 입력 바인딩을 위한 엔진 기본 함수
	virtual void SetupInputComponent() override;

	// 아무 키나 눌렸을 때 실행될 함수
	void OnGlobalInputPressed();

public:
	UFUNCTION()
	void GoBack();

	UFUNCTION() 
	void ShowTitleScreen();

	UFUNCTION() 
	void ShowMainMenuScreen();

	UFUNCTION() 
	void ShowOptionsScreen();

	UFUNCTION()
	void ShowGameStartCamera();

	UFUNCTION()
	void ShowMetaUpgradeScreen();
private:
	void OnMainMenuCameraBlendFinished();
	void OnMetaUpgradeCameraBlendFinished();
	void OnOptionsCameraBlendFinished();

	FTimerHandle MenuTimerHandle;
};
