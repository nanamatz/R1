


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

		// (기존의 머티리얼 링과 텍스트 퍼센트 업데이트 코드 유지)
		if (ProgressMaterial)
		{
			ProgressMaterial->SetScalarParameterValue(FName("Percent"), CurrentVisualProgress);
		}

		if (Text_Percent)
		{
			Text_Percent->SetText(FText::AsNumber(FMath::RoundToInt(CurrentVisualProgress * 100.0f)));
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

		if (Text_Percent)
		{
			Text_Percent->SetText(FText::AsNumber(100));
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
