


#include "UI/System/R1ProgressWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"

void UR1ProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CurrentVisualProgress = 0.0f;
	TargetVisualProgress = 0.0f;

	if (Image_Ring)
	{
		ProgressMaterial = Image_Ring->GetDynamicMaterial();
	}
}

void UR1ProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (CurrentVisualProgress < TargetVisualProgress)
	{
		CurrentVisualProgress = FMath::FInterpTo(CurrentVisualProgress, TargetVisualProgress, InDeltaTime, 3.0f);

		// 링(프로그레스 바) 머티리얼만 갱신. 퍼센트 텍스트는 더 이상 표시하지 않는다.
		if (ProgressMaterial)
		{
			ProgressMaterial->SetScalarParameterValue(FName("Percent"), CurrentVisualProgress);
		}
	}

	if (CurrentVisualProgress >= 0.99f && TargetVisualProgress >= 1.0f && !bHasFiredFinished)
	{
		bHasFiredFinished = true; 

		CurrentVisualProgress = 1.0f;

		if (ProgressMaterial)
		{
			ProgressMaterial->SetScalarParameterValue(FName("Percent"), 1.0f);
		}

		if (OnFinished.IsBound())
		{
			OnFinished.Broadcast();
		}
	}
}

void UR1ProgressWidget::UpdateProgress(float InTargetProgress)
{
	TargetVisualProgress = InTargetProgress;

	if (TargetVisualProgress < 1.0f)
	{
		bHasFiredFinished = false;
	}
}
