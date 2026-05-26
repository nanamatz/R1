


#include "UI/System/R1GameMenuWIdget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "System/R1LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Player/R1PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Map/R1MapGenerator.h"
#include "EngineUtils.h"
#include "UI/R1CommonButton.h"

void UR1GameMenuWIdget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Resume && Button_Resume->CommonButton)
	{
		Button_Resume->CommonButton->OnClicked.AddDynamic(this, &UR1GameMenuWIdget::OnResumeButtonClicked);
	}

	if (Button_Options && Button_Options->CommonButton)
	{
		Button_Options->CommonButton->OnClicked.AddDynamic(this, &UR1GameMenuWIdget::OnOptionsButtonClicked);
	}

	if (Button_Exit && Button_Exit->CommonButton)
	{
		Button_Exit->CommonButton->OnClicked.AddDynamic(this, &UR1GameMenuWIdget::OnExitButtonClicked);
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.AddUObject(this, &UR1GameMenuWIdget::RefreshLocalization);
		}
	}

	RefreshLocalization();
}

void UR1GameMenuWIdget::NativeDestruct()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.RemoveAll(this);
		}
	}
	Super::NativeDestruct();
}

void UR1GameMenuWIdget::RefreshLocalization()
{
	UGameInstance* GI = GetGameInstance();
	UR1LocalizationSubsystem* LocSub = GI ? GI->GetSubsystem<UR1LocalizationSubsystem>() : nullptr;
	if (!LocSub) return;

	if (Text_Paused) Text_Paused->SetText(LocSub->GetText("Label_Paused"));
}

void UR1GameMenuWIdget::OnResumeButtonClicked()
{
	if (AR1PlayerController* PC = Cast<AR1PlayerController>(GetOwningPlayer()))
	{
		PC->OnGameMenuToggle();
	}
}

void UR1GameMenuWIdget::OnOptionsButtonClicked()
{
	if (AR1PlayerController* PC = Cast<AR1PlayerController>(GetOwningPlayer()))
	{
		PC->OnOptionsUIToggle();
	}
}

void UR1GameMenuWIdget::OnExitButtonClicked()
{
	// 0. 나가기 전에 현재 상태를 자동 저장!
	for (TActorIterator<AR1MapGenerator> It(GetWorld()); It; ++It)
	{
		It->TriggerAutoSave();
		break;
	}

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->SetPause(false);
	}
	if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		CameraManager->StartCameraFade(0.0f, 1.0f, 1.0f, FLinearColor::Black, false, true);
	}

	// 1초 대기 후 ExecuteExit 실행
	GetWorld()->GetTimerManager().SetTimer(TransitionTimerHandle, this, &UR1GameMenuWIdget::ExecuteExit, 1.0f, false);
}

void UR1GameMenuWIdget::ExecuteExit()
{
	if (!TitleLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, TitleLevelName);
	}
}