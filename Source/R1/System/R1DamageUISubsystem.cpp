#include "System/R1DamageUISubsystem.h"
#include "UI/R1DamageTextWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void UR1DamageUISubsystem::ShowDamageText(const FR1DamageInfo& DamageInfo)
{
	UR1DamageTextWidget* Widget = GetWidgetFromPool();
	if (Widget)
	{
		Widget->SetDamageInfo(DamageInfo);
		
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport();
		}

		FVector2D ScreenPosition;
		if (UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), DamageInfo.TargetLocation + FVector(0,0,100), ScreenPosition))
		{
			Widget->SetPositionInViewport(ScreenPosition);
		}
	}
}

void UR1DamageUISubsystem::ReturnWidgetToPool(UR1DamageTextWidget* Widget)
{
	if (Widget)
	{
		Widget->RemoveFromParent();
		WidgetPool.Add(Widget);
	}
}

UR1DamageTextWidget* UR1DamageUISubsystem::GetWidgetFromPool()
{
	if (WidgetPool.Num() > 0)
	{
		return WidgetPool.Pop();
	}

	if (DamageWidgetClass)
	{
		return CreateWidget<UR1DamageTextWidget>(GetWorld(), DamageWidgetClass);
	}

	return nullptr;
}
