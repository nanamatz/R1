


#include "UI/System/R1MainMenuWidget.h"
#include "Components/Button.h"
#include "System/R1SaveSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UR1MainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// ==========================================
	// 1. UI 버튼들에 클릭 이벤트 바인딩
	// ==========================================
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

	// ==========================================
	// 2. 세이브 여부에 따른 Continue 버튼 활성화
	// ==========================================
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

void UR1MainMenuWidget::OnNewRunButtonClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UR1SaveSystem* SaveSystem = GameInstance->GetSubsystem<UR1SaveSystem>())
		{
			SaveSystem->DeleteSavedRun();
		}
	}

	// 1층 맵으로 이동
	if (!GameLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, GameLevelName);
	}
}

void UR1MainMenuWidget::OnContinueButtonClicked()
{
	if (!GameLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, GameLevelName);
	}
}

void UR1MainMenuWidget::OnOptionsButtonClicked()
{
	// TODO: 나중에 옵션 창 UI를 띄우는 로직을 여기에 작성하시면 됩니다.
	UE_LOG(LogTemp, Warning, TEXT("[MainMenu] 옵션 버튼 클릭됨!"));
}

void UR1MainMenuWidget::OnExitButtonClicked()
{
	// 게임 완전 종료! (에디터에서는 플레이 모드가 정지됩니다)
	APlayerController* SpecificPlayer = GetOwningPlayer();
	UKismetSystemLibrary::QuitGame(this, SpecificPlayer, EQuitPreference::Quit, false);
}
