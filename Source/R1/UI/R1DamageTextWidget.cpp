#include "UI/R1DamageTextWidget.h"
#include "System/R1DamageUISubsystem.h" 

void UR1DamageTextWidget::SetDamageInfo(const FR1DamageInfo& Info)
{
	OnSetDamageInfo(Info);
}

void UR1DamageTextWidget::ReturnToPool()
{
	if (UWorld* World = GetWorld())
	{
		if (UR1DamageUISubsystem* DamageSS = World->GetSubsystem<UR1DamageUISubsystem>())
		{
			DamageSS->ReturnWidgetToPool(this);
		}
	}
}

void UR1DamageTextWidget::HandleAnimationFinished()
{
	ReturnToPool();
}
