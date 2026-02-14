


#include "UI/R1ExpBarWidget.h"
#include "Character/R1Player.h"
#include "Components/ProgressBar.h"

UR1ExpBarWidget::UR1ExpBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TargetPercent = 0.f;
}

void UR1ExpBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AR1Player* Player = Cast<AR1Player>(GetOwningPlayerPawn());
	if (Player)
	{
		Player->OnExpChanged.AddDynamic(this, &UR1ExpBarWidget::UpdateExpBar);

		// 팁: UI가 처음 켜졌을 때 현재 경험치 값으로 TargetPercent를 맞춰줍니다.
		// (Player에 GetCurrentExpRatio() 같은 함수가 있다면 사용하세요)
		// TargetPercent = Player->GetCurrentExpRatio(); 
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("R1ExpBarWidget: Failed to find AR1Player during NativeConstruct!"));
	}
	// ExpBar 컴포넌트 유효성 검사 로그 추가
	if (!ExpBar)
	{
		UE_LOG(LogTemp, Error, TEXT("R1ExpBarWidget: ExpBar component is missing!"));
	}

	UpdateExpBar(TargetPercent); // 초기값 설정
}

void UR1ExpBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ExpBar)
	{
		float CurExpRatio = ExpBar->GetPercent();

		// 2. 현재 비율과 목표 비율이 다를 때만(오차 범위 내) 보간 및 UI 갱신 수행
		if (!FMath::IsNearlyEqual(CurExpRatio, TargetPercent, 0.001f))
		{
			// 목표 값을 향해 부드럽게 이동
			float NextPercent = FMath::FInterpTo(CurExpRatio, TargetPercent, InDeltaTime, 2.0f);
			ExpBar->SetPercent(NextPercent);
		}
	}
}

void UR1ExpBarWidget::UpdateExpBar(float Ratio)
{
	TargetPercent = Ratio;
}
