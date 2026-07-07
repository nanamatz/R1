


#include "UI/PlayerInfo/R1PlayerInfoWidget.h"
#include "R1LogChannels.h"
#include "UI/PlayerInfo/R1HpOrbWidget.h"
#include "UI/PlayerInfo/R1MpOrbWidget.h"
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
		UE_LOG(LogR1, Warning, TEXT("R1PlayerInfoWidget: Btn_HpOrbHover not bound — HP text hover will not work."));
	}

	if (!HpOrbWidget)
	{
		UE_LOG(LogR1, Warning, TEXT("R1PlayerInfoWidget: HpOrbWidget not bound — cannot toggle HP text."));
	}

	if (Btn_MpOrbHover)
	{
		Btn_MpOrbHover->OnHovered.AddUniqueDynamic(this, &UR1PlayerInfoWidget::OnMpOrbHitBoxHovered);
		Btn_MpOrbHover->OnUnhovered.AddUniqueDynamic(this, &UR1PlayerInfoWidget::OnMpOrbHitBoxUnhovered);
	}
	else
	{
		UE_LOG(LogR1, Warning, TEXT("R1PlayerInfoWidget: Btn_MpOrbHover not bound — MP text hover will not work."));
	}

	if (!MpOrbWidget)
	{
		UE_LOG(LogR1, Warning, TEXT("R1PlayerInfoWidget: MpOrbWidget not bound — cannot toggle MP text."));
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

void UR1PlayerInfoWidget::OnMpOrbHitBoxHovered()
{
	if (MpOrbWidget)
	{
		MpOrbWidget->SetMpTextVisible(true);
	}
}

void UR1PlayerInfoWidget::OnMpOrbHitBoxUnhovered()
{
	if (MpOrbWidget)
	{
		MpOrbWidget->SetMpTextVisible(false);
	}
}
