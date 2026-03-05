


#include "UI/PlayerInfo/R1HpOrbWidget.h"
#include "Components/Image.h"
#include "Character/R1Player.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

void UR1HpOrbWidget::NativeConstruct()
{
	Super::NativeConstruct();

    // 1. 이미지 위젯이 있다면, 동적 머티리얼을 생성하고 저장해둡니다.
    if (HpOrb)
    {
        // 이미지에 할당된 머티리얼을 동적으로 바꿀 수 있게 가져옵니다.
        DynamicOrbMaterial = HpOrb->GetDynamicMaterial();
    }
    AR1Player* Player = Cast<AR1Player>(GetOwningPlayerPawn());
    //AR1Player* Player = Cast<AR1Player>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (Player)
    {
        Player->OnHpChanged.AddDynamic(this, &UR1HpOrbWidget::UpdateHpOrb);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("R1HpOrbWidget: Failed to find AR1Player during NativeConstruct!"));
    }

}

void UR1HpOrbWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
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

void UR1HpOrbWidget::UpdateHpOrb(float Ratio)
{
    TargetPercent = Ratio;
}
