


#include "Player/R1MainMenuController.h"
#include "Kismet/GameplayStatics.h"
#include "UI/System/R1TitleScreenWidget.h"
#include "UI/System/R1TitleWidget.h"
#include "UI/System/R1MainMenuWidget.h"
#include "Kismet/KismetSystemLibrary.h"

void AR1MainMenuController::BeginPlay()
{
	Super::BeginPlay();
	// 레벨에 배치된 카메라들을 찾아서 포인터에 저장
	bShowMouseCursor = true; // 메뉴니까 마우스 켜기

	// 1. 레벨에 있는 모든 액터를 뒤져서 태그가 일치하는 카메라를 찾습니다.
	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsWithTag(this, FName("Camera_Title"), FoundCameras);
	if (FoundCameras.Num() > 0)
	{
		TitleCamera = FoundCameras[0];
	}

	UGameplayStatics::GetAllActorsWithTag(this, FName("Camera_Options"), FoundCameras);
	if (FoundCameras.Num() > 0)
	{
		OptionsCamera = FoundCameras[0];
	}

	UGameplayStatics::GetAllActorsWithTag(this, FName("Camera_MainMenu"), FoundCameras);
	if (FoundCameras.Num() > 0)
	{
		MainMenuCamera = FoundCameras[0];
	}

	UGameplayStatics::GetAllActorsWithTag(this, FName("Camera_GameStart"), FoundCameras);
	if (FoundCameras.Num() > 0)
	{
		GameStartCamera = FoundCameras[0];
	}

	// 2. 게임 시작 시 타이틀 카메라로 시점을 즉시 고정합니다.
	if (TitleCamera)
	{
		SetViewTarget(TitleCamera);
	}

	if (TitleScreenWidgetClass)
	{
		TitleScreenWidget = CreateWidget<UR1TitleScreenWidget>(this, TitleScreenWidgetClass);
		if (TitleScreenWidget)
		{
			TitleScreenWidget->AddToViewport();
		}
	}

	ShowTitleScreen();

	if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		// 1.0(까망) -> 0.0(투명) 으로 1초 동안 변환
		CameraManager->StartCameraFade(1.0f, 0.0f, 1.0f, FLinearColor::Black, false, false);
	}
}

void AR1MainMenuController::ShowTitleScreen()
{
	CurrentMenuState = EMenuState::Title;

	if (TitleScreenWidget)
	{
		TitleScreenWidget->SwitchToTitle();

		FInputModeGameAndUI InputModeData;
		InputModeData.SetHideCursorDuringCapture(false); // 클릭 시 마우스 숨김 방지

		if (TitleScreenWidget->WBP_Title)
		{
			InputModeData.SetWidgetToFocus(TitleScreenWidget->WBP_Title->TakeWidget());
		}
		SetInputMode(InputModeData);
	}

	if (TitleCamera)
	{
		SetViewTargetWithBlend(TitleCamera, 1.5f, EViewTargetBlendFunction::VTBlend_Cubic, 2.0f);
	}
}

void AR1MainMenuController::ShowMainMenuScreen()
{
	CurrentMenuState = EMenuState::MainMenu;

	// 1. 카메라 이동 시작 (1.5초)
	if (MainMenuCamera)
	{
		SetViewTargetWithBlend(MainMenuCamera, 1.5f, EViewTargetBlendFunction::VTBlend_Cubic, 2.0f);
	}

	// 2. 현재 떠있는 UI(타이틀 화면)는 즉시 숨기기
	if (TitleScreenWidget && TitleScreenWidget->WBP_Title)
	{
		TitleScreenWidget->WBP_Title->SetVisibility(ESlateVisibility::Hidden);
	}

	// 3. 1.5초 뒤에 메인 메뉴 UI를 띄우도록 타이머 설정
	GetWorldTimerManager().SetTimer(MenuTimerHandle, this, &AR1MainMenuController::OnMainMenuCameraBlendFinished, 1.5f, false);
}

void AR1MainMenuController::OnMainMenuCameraBlendFinished()
{
	if (TitleScreenWidget)
	{
		TitleScreenWidget->SwitchToMainMenu();

		FInputModeUIOnly InputModeData;
		if (TitleScreenWidget->WBP_MainMenu)
		{
			InputModeData.SetWidgetToFocus(TitleScreenWidget->WBP_MainMenu->TakeWidget());
		}
		SetInputMode(InputModeData);
	}
}

void AR1MainMenuController::ShowOptionsScreen()
{
	CurrentMenuState = EMenuState::Options;

	if (TitleScreenWidget)
	{
		TitleScreenWidget->SwitchToOptions();
	}
	if (OptionsCamera)
	{
		SetViewTargetWithBlend(OptionsCamera, 1.5f, EViewTargetBlendFunction::VTBlend_Cubic, 2.0f);
	}

}

void AR1MainMenuController::ShowGameStartCamera()
{
	if (GameStartCamera)
	{
		SetViewTargetWithBlend(GameStartCamera, 1.5f, EViewTargetBlendFunction::VTBlend_Cubic, 2.0f);
	}
}

void AR1MainMenuController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindKey(EKeys::AnyKey, IE_Pressed, this, &AR1MainMenuController::OnGlobalInputPressed);
}

void AR1MainMenuController::OnGlobalInputPressed()
{
	if (WasInputKeyJustPressed(EKeys::Escape))
	{
		if (CurrentMenuState == EMenuState::Title)
		{
			// 타이틀 화면에서 ESC 누르면 게임 종료!
			UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
		}
		else
		{
			// 메인 메뉴나 옵션 등 다른 화면이면 뒤로 가기!
			GoBack();
		}
		return; // ESC 처리를 했으니 여기서 함수 끝
	}

	// 2. ESC가 아닌 '아무 키'나 '마우스 클릭'일 때
	if (CurrentMenuState == EMenuState::Title)
	{
		// 타이틀 화면이라면 메인 메뉴로 넘어갑니다!
		ShowMainMenuScreen();
	}
}

void AR1MainMenuController::GoBack()
{
	switch (CurrentMenuState)
	{
	case EMenuState::Options:
		ShowMainMenuScreen(); // 옵션에서는 메인 메뉴로
		break;
	case EMenuState::MainMenu:
		ShowTitleScreen();    // 메인 메뉴에서는 타이틀로
		break;
	case EMenuState::Title:
		// 타이틀에서는 뒤로 갈 곳이 없으니 무시하거나, 게임 종료 처리
		break;
	}
}