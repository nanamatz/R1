


#include "UI/PlayerInfo/R1MpOrbWidget.h"
#include "R1LogChannels.h"
#include "UI/PlayerInfo/R1MpTextUI.h"
#include "Components/Image.h"
#include "Character/R1Player.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

void UR1MpOrbWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 1. 이미지 위젯이 있다면, 동적 머티리얼을 생성하고 저장해둡니다.
    if (MpOrb)
    {
        // 이미지에 할당된 머티리얼을 동적으로 바꿀 수 있게 가져옵니다.
        DynamicOrbMaterial = MpOrb->GetDynamicMaterial();
    }
    AR1Player* Player = Cast<AR1Player>(GetOwningPlayerPawn());

    if (Player)
    {
        Player->OnMpChanged.AddDynamic(this, &UR1MpOrbWidget::UpdateMpOrb);
    }
    else
    {
        UE_LOG(LogR1, Error, TEXT("R1HpOrbWidget: Failed to find AR1Player during NativeConstruct!"));
    }

    // 기본은 숨김 — 호버 시에만 표시
    if (MpTextUI)
    {
        MpTextUI->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UR1MpOrbWidget::SetMpTextVisible(bool bVisible)
{
    if (MpTextUI)
    {
        MpTextUI->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UR1MpOrbWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (DynamicOrbMaterial)
    {
        // 현재 값을 가져옵니다.
        float CurrentScalar;
        DynamicOrbMaterial->GetScalarParameterValue(MaterialParamName, CurrentScalar);

        // 목표 값(targetpercent)을 향해 부드럽게 이동
        float NextScalar = FMath::FInterpTo(CurrentScalar, TargetPercent, InDeltaTime, 1.0f);

        DynamicOrbMaterial->SetScalarParameterValue(MaterialParamName, NextScalar);
    }
}

void UR1MpOrbWidget::UpdateMpOrb(float Ratio)
{
    TargetPercent = Ratio;
}
