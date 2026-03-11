


#include "UI/System/R1GameMenuWIdget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Player/R1PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

void UR1GameMenuWIdget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Resume)
	{
		Button_Resume->OnClicked.AddDynamic(this, &UR1GameMenuWIdget::OnResumeButtonClicked);
	}

	if (Button_Options)
	{
		Button_Options->OnClicked.AddDynamic(this, &UR1GameMenuWIdget::OnOptionsButtonClicked);
	}

	if (Button_Exit)
	{
		Button_Exit->OnClicked.AddDynamic(this, &UR1GameMenuWIdget::OnExitButtonClicked);
	}
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
	UE_LOG(LogTemp, Warning, TEXT("[GameMenu] 옵션 버튼 클릭됨!"));
}

void UR1GameMenuWIdget::OnExitButtonClicked()
{
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