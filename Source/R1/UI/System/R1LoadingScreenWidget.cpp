


#include "UI/System/R1LoadingScreenWidget.h"
#include "UI/System/R1ProgressWidget.h"

void UR1LoadingScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 내부에 있는 프로그레스 바가 100% 다 찼다고 하면, 이 위젯의 함수를 실행하도록 연결!
	if (WBP_ProgressBar)
	{
		WBP_ProgressBar->OnFinished.RemoveDynamic(this, &UR1LoadingScreenWidget::HandleVisualFinished);
		WBP_ProgressBar->OnFinished.AddDynamic(this, &UR1LoadingScreenWidget::HandleVisualFinished);
	}
}

void UR1LoadingScreenWidget::HandleVisualFinished()
{
	if (OnSceneFinished.IsBound())
	{
		OnSceneFinished.Broadcast();
	}
}

void UR1LoadingScreenWidget::SetProgress(float InPercent)
{
	if (WBP_ProgressBar)
	{
		WBP_ProgressBar->UpdateProgress(InPercent); // 여기서 R1ProgressWidget의 Tick 로직이 작동함
	}
}
