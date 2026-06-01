


#include "UI/PlayerInfo/R1PlayerInfoWidget.h"
#include "UI/PlayerInfo/R1HpOrbWidget.h"
#include "Components/Button.h"

void UR1PlayerInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_HpOrbHover)
	{
		Btn_HpOrbHover->OnHovered.AddUniqueDynamic(this, &UR1PlayerInfoWidget::OnHpOrbHitBoxHovered);
		Btn_HpOrbHover->OnUnhovered.AddUniqueDynamic(this, &UR1PlayerInfoWidget::OnHpOrbHitBoxUnhovered);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("R1PlayerInfoWidget: Btn_HpOrbHover not bound — HP text hover will not work."));
	}

	if (!HpOrbWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("R1PlayerInfoWidget: HpOrbWidget not bound — cannot toggle HP text."));
	}
}

void UR1PlayerInfoWidget::OnHpOrbHitBoxHovered()
{
	if (HpOrbWidget)
	{
		HpOrbWidget->SetHpTextVisible(true);
	}
}

void UR1PlayerInfoWidget::OnHpOrbHitBoxUnhovered()
{
	if (HpOrbWidget)
	{
		HpOrbWidget->SetHpTextVisible(false);
	}
}
