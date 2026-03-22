


#include "UI/System/R1MainMenuWidget.h"
#include "Components/Button.h"
#include "System/R1SaveSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "R1GameMenuWIdget.h"
#include "Player/R1MainMenuController.h"

void UR1MainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	if (Button_NewRun)
	{
		Button_NewRun->OnClicked.AddDynamic(this, &UR1MainMenuWidget::OnNewRunButtonClicked);
	}

	if (Button_Continue)
	{
		Button_Continue->OnClicked.AddDynamic(this, &UR1MainMenuWidget::OnContinueButtonClicked);
	}

	if (Button_Options)
	{
		Button_Options->OnClicked.AddDynamic(this, &UR1MainMenuWidget::OnOptionsButtonClicked);
	}

	if (Button_Exit)
	{
		Button_Exit->OnClicked.AddDynamic(this, &UR1MainMenuWidget::OnExitButtonClicked);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UR1SaveSystem* SaveSystem = GameInstance->GetSubsystem<UR1SaveSystem>())
		{
			// 세이브 파일이 있으면 true, 없으면 false
			bool bHasSave = SaveSystem->HasSavedRun();

			if (Button_Continue)
			{
				Button_Continue->SetIsEnabled(bHasSave);
			}
		}
	}
}

FReply UR1MainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		if (AR1MainMenuController* MenuPC = Cast<AR1MainMenuController>(GetOwningPlayer()))
		{
			// 컨트롤러의 만능 뒤로가기 호출!
			MenuPC->GoBack();

			// 우리가 입력을 처리했으니, 엔진의 다른 곳으로 키보드 입력이 넘어가지 않게 막음
			return FReply::Handled();
		}
	}

	// ESC가 아니라면 부모 클래스의 원래 로직을 실행
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UR1MainMenuWidget::OnNewRunButtonClicked()
{
	StartGameTransition(true);
}

void UR1MainMenuWidget::OnContinueButtonClicked()
{
	StartGameTransition(false);
}

void UR1MainMenuWidget::OnOptionsButtonClicked()
{
	if (AR1MainMenuController* MenuPC = Cast<AR1MainMenuController>(GetOwningPlayer()))
	{
		MenuPC->ShowOptionsScreen();
	}
}

void UR1MainMenuWidget::OnExitButtonClicked()
{
	// 게임 완전 종료! (에디터에서는 플레이 모드가 정지됩니다)
	APlayerController* SpecificPlayer = GetOwningPlayer();
	UKismetSystemLibrary::QuitGame(this, SpecificPlayer, EQuitPreference::Quit, false);
}

void UR1MainMenuWidget::StartGameTransition(bool bIsNewRun)
{
	bIsNewRunPending = bIsNewRun;

	SetVisibility(ESlateVisibility::Hidden);

	float TransitionDuration = 2.0f; // 연출에 걸리는 총 시간 (2초)

	// 컨트롤러에게 '시작 지점으로 카메라를 날려라!' 라고 명령합니다.
	AR1MainMenuController* MenuPC = Cast<AR1MainMenuController>(GetOwningPlayer());
	if (MenuPC)
	{
		MenuPC->ShowGameStartCamera();
	}

	// 카메라가 날아가는 동안 화면을 서서히 까맣게(Fade Out) 만듭니다.
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (CameraManager)
	{
		CameraManager->StartCameraFade(0.0f, 1.0f, TransitionDuration, FLinearColor::Black, false, true);
	}

	// 2초(TransitionDuration) 뒤에 진짜로 맵을 로드하는 함수를 실행하도록 예약합니다!
	GetWorld()->GetTimerManager().SetTimer(LevelLoadTimerHandle, this, &UR1MainMenuWidget::ExecuteLevelLoad, TransitionDuration, false);
}

void UR1MainMenuWidget::ExecuteLevelLoad()
{
	if (bIsNewRunPending)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UR1SaveSystem* SaveSystem = GameInstance->GetSubsystem<UR1SaveSystem>())
			{
				SaveSystem->DeleteSavedRun();
			}
		}
	}

	if (!GameLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, GameLevelName);
	}
}
