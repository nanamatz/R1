


#include "UI/PlayerInfo/PlayerInfoSceneWidget.h"
#include "UI/PlayerInfo/R1HpOrbWidget.h"
#include "Components/Button.h"

void UPlayerInfoSceneWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_HpOrbHover)
	{
		Btn_HpOrbHover->OnHovered.AddUniqueDynamic(this, &UPlayerInfoSceneWidget::OnHpOrbHitBoxHovered);
		Btn_HpOrbHover->OnUnhovered.AddUniqueDynamic(this, &UPlayerInfoSceneWidget::OnHpOrbHitBoxUnhovered);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerInfoSceneWidget: Btn_HpOrbHover not bound — HP text hover will not work."));
	}

	if (!HpOrbWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerInfoSceneWidget: HpOrbWidget not bound — cannot toggle HP text."));
	}
}

void UPlayerInfoSceneWidget::OnHpOrbHitBoxHovered()
{
	if (HpOrbWidget)
	{
		HpOrbWidget->SetHpTextVisible(true);
	}
}

void UPlayerInfoSceneWidget::OnHpOrbHitBoxUnhovered()
{
	if (HpOrbWidget)
	{
		HpOrbWidget->SetHpTextVisible(false);
	}
}
