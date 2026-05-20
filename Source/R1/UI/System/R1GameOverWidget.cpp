
#include "UI/System/R1GameOverWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "System/R1SaveSystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/R1CommonButton.h"
#include "Item/R1InventorySubsystem.h"

void UR1GameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Retry && Button_Retry->CommonButton)
	{
		Button_Retry->CommonButton->OnClicked.AddDynamic(this, &UR1GameOverWidget::OnRetryClicked);
	}

	if (Button_Exit && Button_Exit->CommonButton)
	{
		Button_Exit->CommonButton->OnClicked.AddDynamic(this, &UR1GameOverWidget::OnExitClicked);
	}
}

void UR1GameOverWidget::OnRetryClicked()
{
	if (UWorld* World = GetWorld())
	{
		// 1. 세이브 데이터 삭제
		if (UR1SaveSystem* SaveSystem = World->GetGameInstance()->GetSubsystem<UR1SaveSystem>())
		{
			SaveSystem->DeleteSavedRun();
		}

		// 2. 인벤토리 초기화 (데이터 상으로는 Clear 후 기본 아이템 지급)
		if (UR1InventorySubsystem* InventorySubsystem = World->GetSubsystem<UR1InventorySubsystem>())
		{
			InventorySubsystem->ClearInventory();
		}

		if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
		{
			CameraManager->StartCameraFade(0.0f, 1.0f, 1.0f, FLinearColor::Black, false, true);
		}

		// 2. 1초 뒤에 ExecuteRestart 함수를 실행합니다!
		GetWorld()->GetTimerManager().SetTimer(TransitionTimerHandle, this, &UR1GameOverWidget::ExecuteRestart, 1.0f, false);
	}
}

void UR1GameOverWidget::OnExitClicked()
{
	if (UWorld* World = GetWorld())
	{
		// 사망 후 타이틀로 나갈 때도 현재 런 세이브 데이터 삭제
		if (UR1SaveSystem* SaveSystem = World->GetGameInstance()->GetSubsystem<UR1SaveSystem>())
		{
			SaveSystem->DeleteSavedRun();
		}
	}
	if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		CameraManager->StartCameraFade(0.0f, 1.0f, 1.0f, FLinearColor::Black, false, true);
	}

	GetWorld()->GetTimerManager().SetTimer(TransitionTimerHandle, this, &UR1GameOverWidget::ExecuteExit, 1.0f, false);
}

void UR1GameOverWidget::ExecuteRestart()
{
	if (UWorld* World = GetWorld())
	{
		FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(World);
		UGameplayStatics::OpenLevel(World, FName(*CurrentLevelName));
	}
}

void UR1GameOverWidget::ExecuteExit()
{
	if (!TitleLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, TitleLevelName);
	}
}
