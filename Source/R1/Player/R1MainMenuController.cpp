


#include "Player/R1MainMenuController.h"
#include "Kismet/GameplayStatics.h"
#include "UI/System/R1TitleScreenWidget.h"
#include "UI/System/R1TitleWidget.h"
#include "UI/System/R1MainMenuWidget.h"
#include "UI/System/R1TitleOptionsMenuWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Progression/R1MetaUpgradeWidget.h"

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

	UGameplayStatics::GetAllActorsWithTag(this, FName("Camera_MetaUpgrade"), FoundCameras);
	if (FoundCameras.Num() > 0)
	{
		MetaUpgradeCamera = FoundCameras[0];
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

			// 옵션 창의 닫기 요청 이벤트 바인딩
			if (TitleScreenWidget->WBP_OptionWidget)
			{
				TitleScreenWidget->WBP_OptionWidget->OnCloseRequested.AddDynamic(this, &AR1MainMenuController::GoBack);
			}
		}
	}

	ShowTitleScreen();

	if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
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

	if (MainMenuCamera)
	{
		SetViewTargetWithBlend(MainMenuCamera, 1.5f, EViewTargetBlendFunction::VTBlend_Cubic, 2.0f);
	}

	if (TitleScreenWidget)
	{
		if (TitleScreenWidget->WBP_Title)
		{
			TitleScreenWidget->WBP_Title->SetVisibility(ESlateVisibility::Hidden);
		}

		if (TitleScreenWidget->WBP_UpgradeWidget)
		{
			TitleScreenWidget->WBP_UpgradeWidget->SetVisibility(ESlateVisibility::Hidden);
		}

		if (TitleScreenWidget->WBP_OptionWidget)
		{
			TitleScreenWidget->WBP_OptionWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

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

void AR1MainMenuController::OnMetaUpgradeCameraBlendFinished()
{
	if (TitleScreenWidget)
	{
		TitleScreenWidget->SwitchToMetaUpgrade(); // 이동이 끝난 후 UI 켜기!

		FInputModeUIOnly InputModeData;
		if (TitleScreenWidget->WBP_UpgradeWidget)
		{
			InputModeData.SetWidgetToFocus(TitleScreenWidget->WBP_UpgradeWidget->TakeWidget());
		}
		SetInputMode(InputModeData);
	}
}

void AR1MainMenuController::OnOptionsCameraBlendFinished()
{
	if (TitleScreenWidget)
	{
		TitleScreenWidget->SwitchToOptions(); // 이동 끝난 후 띄움!
	}
}

void AR1MainMenuController::ShowOptionsScreen()
{
	CurrentMenuState = EMenuState::Options;

	if (TitleScreenWidget)
	{
		if (TitleScreenWidget->WBP_MainMenu)
		{
			TitleScreenWidget->WBP_MainMenu->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (OptionsCamera)
	{
		SetViewTargetWithBlend(OptionsCamera, 1.5f, EViewTargetBlendFunction::VTBlend_Cubic, 2.0f);
	}

	GetWorldTimerManager().SetTimer(MenuTimerHandle, this, &AR1MainMenuController::OnOptionsCameraBlendFinished, 1.5f, false);
}

void AR1MainMenuController::ShowGameStartCamera()
{
	if (GameStartCamera)
	{
		SetViewTargetWithBlend(GameStartCamera, 1.5f, EViewTargetBlendFunction::VTBlend_Cubic, 2.0f);
	}
}

void AR1MainMenuController::ShowMetaUpgradeScreen()
{
	CurrentMenuState = EMenuState::MetaUpgrade;

	if (TitleScreenWidget)
	{
		if (TitleScreenWidget->WBP_MainMenu)
		{
			TitleScreenWidget->WBP_MainMenu->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (MetaUpgradeCamera)
	{
		SetViewTargetWithBlend(MetaUpgradeCamera, 1.5f, EViewTargetBlendFunction::VTBlend_Cubic, 2.0f);
	}

	GetWorldTimerManager().SetTimer(MenuTimerHandle, this, &AR1MainMenuController::OnMetaUpgradeCameraBlendFinished, 1.5f, false);
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
		else if (CurrentMenuState == EMenuState::Options)
		{
			// 옵션 화면에서 ESC 누르면 위젯의 Cancel 로직(모달 체크 등)을 직접 호출
			if (TitleScreenWidget && TitleScreenWidget->WBP_OptionWidget)
			{
				TitleScreenWidget->WBP_OptionWidget->OnCancelButtonClicked();
			}
		}
		else
		{
			// 메인 메뉴 등 다른 화면이면 뒤로 가기!
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
	case EMenuState::MetaUpgrade:
		ShowMainMenuScreen();         
		break;
	case EMenuState::Options:
		ShowMainMenuScreen();
		break;
	case EMenuState::MainMenu:
		ShowTitleScreen(); 
		break;
	case EMenuState::Title:
		break;
	}
}