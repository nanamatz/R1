


#include "System/R1LoadingSubSystem.h"
#include "UI/System/R1ProgressWidget.h"
#include "Map/R1MapGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "UI/System/R1LoadingScreenWidget.h"

void UR1LoadingSubSystem::ShowLoadingScreen(TSubclassOf<UR1LoadingScreenWidget> WidgetClass, AR1MapGenerator* MapGenerator)
{
	if (!WidgetClass) return;

	// 1. 위젯이 없다면 생성하고, 뷰포트 최상단(999)에 띄웁니다.
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
}

void UR1LoadingSubSystem::OnProgressUpdated(float CurrentProgress)
{
	if (LoadingWidget)
	{
		LoadingWidget->SetProgress(CurrentProgress);
	}

	//// 2. 진행률이 100% (1.0f)에 도달했다면?
	//if (CurrentProgress >= 1.0f)
	//{
	//	// 유저가 100%를 눈으로 확인할 수 있게 0.5초 뒤에 화면을 걷어냅니다.
	//	FTimerHandle TimerHandle;
	//	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UR1LoadingSubSystem::HideLoadingScreen, 0.5f, false);
	//}
}

void UR1LoadingSubSystem::HideLoadingScreen()
{
	if (LoadingWidget)
	{
		LoadingWidget->RemoveFromParent();
		LoadingWidget = nullptr;
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
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UR1LoadingSubSystem::HideLoadingScreen, 0.7f, false);
}
