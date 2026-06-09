


#include "System/R1LoadingSubSystem.h"
#include "UI/System/R1ProgressWidget.h"
#include "Map/R1MapGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "UI/System/R1LoadingScreenWidget.h"
#include "Player/R1MainMenuController.h"
#include "Player/R1PlayerController.h"

void UR1LoadingSubSystem::ShowLoadingScreen(TSubclassOf<UR1LoadingScreenWidget> WidgetClass, AR1MapGenerator* MapGenerator)
{
	if (!WidgetClass) return;

	bSceneDone = false;
	bContentReady = false;

	if (!LoadingWidget)
	{
		LoadingWidget = CreateWidget<UR1LoadingScreenWidget>(GetWorld(), WidgetClass);
	}

	if (LoadingWidget && !LoadingWidget->IsInViewport())
	{
		LoadingWidget->AddToViewport(999);
	}

	if (LoadingWidget)
	{
		LoadingWidget->OnSceneFinished.RemoveDynamic(this, &UR1LoadingSubSystem::OnVisualsCompleted);
		LoadingWidget->OnSceneFinished.AddDynamic(this, &UR1LoadingSubSystem::OnVisualsCompleted);
	}
	// 2. 맵 제너레이터의 방송국(Delegate)에 이 서브시스템의 귀를 연결합니다!
	if (MapGenerator)
	{
		// 중복 등록 방지를 위해 지웠다가 다시 추가 (AddUniqueDynamic 역할)
		MapGenerator->OnGenerateProgressUpdated.RemoveDynamic(this, &UR1LoadingSubSystem::OnProgressUpdated);
		MapGenerator->OnGenerateProgressUpdated.AddDynamic(this, &UR1LoadingSubSystem::OnProgressUpdated);
	}

	if (UWorld* World = GetWorld())
	{
		// 현재 월드의 첫 번째 플레이어 컨트롤러를 가져옵니다.
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (AR1PlayerController* R1PC = Cast<AR1PlayerController>(PC))
			{
				// 1. 하던 행동과 과거의 목적지를 완전히 지워버립니다.
				R1PC->ResetMovementState();

				// 2. 입력 모드를 'UI 전용(UI Only)'으로 바꿔서 월드 클릭(이동/공격)을 원천 차단합니다!
				//R1PC->UpdateInputMode(true);

				// (선택) 만약 UI 전용 모드로도 불안하다면, 엔진 기본 함수로 입력을 아예 막아버릴 수도 있습니다.
				R1PC->DisableInput(R1PC);
			}
		}
	}
	
}

void UR1LoadingSubSystem::OnProgressUpdated(float CurrentProgress)
{
	if (LoadingWidget)
	{
		LoadingWidget->SetProgress(CurrentProgress);
	}
}

void UR1LoadingSubSystem::HideLoadingScreen()
{
	if (LoadingWidget)
	{
		LoadingWidget->RemoveFromParent();
		LoadingWidget = nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (AR1PlayerController* R1PC = Cast<AR1PlayerController>(PC))
			{
				// 1. 로딩이 끝났으니 다시 '게임 및 UI(Game And UI)' 모드로 돌려놓아 이동/공격을 허용합니다!
				//R1PC->UpdateInputMode(false);

				 R1PC->EnableInput(R1PC);
			}
		}
	}

	if (APlayerCameraManager* CamManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
	{
		CamManager->StartCameraFade(1.0f, 0.0f, 1.0f, FLinearColor::Black, false, false);
	}

	if (OnLoadingScreenHidden.IsBound())
	{
		OnLoadingScreenHidden.Broadcast();
	}
}

void UR1LoadingSubSystem::OnVisualsCompleted()
{
	bSceneDone = true;
	TryHideLoadingScreen();
}

void UR1LoadingSubSystem::NotifyContentReady()
{
	bContentReady = true;
	TryHideLoadingScreen();
}

void UR1LoadingSubSystem::TryHideLoadingScreen()
{
	if (!bSceneDone || !bContentReady) return;
	if (!LoadingWidget) return; // 이미 내려갔으면 중복 방지

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UR1LoadingSubSystem::HideLoadingScreen, 0.7f, false);
}
