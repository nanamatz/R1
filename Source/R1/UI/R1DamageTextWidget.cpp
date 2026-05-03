#include "UI/R1DamageTextWidget.h"
// We'll forward declare or include the subsystem here. 
// For now, since it doesn't exist, we'll leave the implementation of ReturnToPool with a comment as requested.
#include "System/R1DamageUISubsystem.h" 

void UR1DamageTextWidget::SetDamageInfo(const FR1DamageInfo& Info)
{
	OnSetDamageInfo(Info);
}

void UR1DamageTextWidget::ReturnToPool()
{
    // Since UR1DamageUISubsystem doesn't exist yet, this will fail to compile if not commented.
    /*
	if (UWorld* World = GetWorld())
	{
		if (UR1DamageUISubsystem* DamageSS = World->GetSubsystem<UR1DamageUISubsystem>())
		{
			DamageSS->ReturnWidgetToPool(this);
		}
	}
    */
}

void UR1DamageTextWidget::HandleAnimationFinished()
{
	ReturnToPool();
}
