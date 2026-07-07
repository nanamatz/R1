


#include "UI/PlayerInfo/R1ExpBarWidget.h"
#include "R1LogChannels.h"
#include "Player/R1PlayerState.h"
#include "Components/ProgressBar.h"

UR1ExpBarWidget::UR1ExpBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UR1ExpBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	AR1PlayerState* PS = GetOwningPlayerState<AR1PlayerState>();
	if (PS)
	{
		// 델리게이트 바인딩
		PS->OnExpChanged.AddDynamic(this, &UR1ExpBarWidget::UpdateExpBar);

		if (ExpBar)
		{
			// 1. 플레이어(또는 AttributeSet)로부터 실제 세이브된/현재 경험치 비율을 가져옵니다.
			float SavedRatio = PS->GetCurrentExpRatio();

			// 2. 애니메이션 없이 즉시 바를 채워줍니다. (로드 직후 0에서 차오르는 현상 방지)
			ExpBar->SetPercent(SavedRatio);

			// 3. 큐를 비우고 현재 비율을 초기 목표로 세팅합니다.
			TargetPercentQueue.Empty();
			TargetPercentQueue.Add(SavedRatio);
		}
	}
	else
	{
		UE_LOG(LogR1, Error, TEXT("R1ExpBarWidget: Failed to find AR1PlayerState during NativeConstruct!"));
	}

	if (!ExpBar)
	{
		UE_LOG(LogR1, Error, TEXT("R1ExpBarWidget: ExpBar component is missing!"));
	}
}

void UR1ExpBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ExpBar && TargetPercentQueue.Num() > 0)
	{
		float CurrentTarget = TargetPercentQueue[0];
		float CurExpRatio = ExpBar->GetPercent();

		// 목표 값에 아직 도달하지 않은 경우 보간 (오차 범위 0.005f로 약간 넉넉하게)
		if (!FMath::IsNearlyEqual(CurExpRatio, CurrentTarget, 0.005f))
		{
			float NextPercent = FMath::FInterpTo(CurExpRatio, CurrentTarget, InDeltaTime, 5.0f); // 2.0은 너무 느릴 수 있어 5.0으로 상향
			ExpBar->SetPercent(NextPercent);
		}
		else
		{
			// 목표 값에 도달했을 때 강제로 목표치에 딱 맞춤
			ExpBar->SetPercent(CurrentTarget);

			// 도달한 목표가 1.0(끝까지 참)이었다면 바를 0으로 초기화 (Wrap-around 핵심!)
			if (CurrentTarget >= 1.0f)
			{
				ExpBar->SetPercent(0.0f);
			}

			// 완료된 목표는 큐에서 제거하여 다음 목표를 향하게 함
			TargetPercentQueue.RemoveAt(0);
		}
	}
}

void UR1ExpBarWidget::UpdateExpBar(float Ratio)
{
	// 1. 경험치가 꽉 찬 경우 (Ratio가 1.0 이상)
	if (Ratio >= 1.0f)
	{
		TargetPercentQueue.Add(1.0f); // 바를 끝까지 채우라는 목표를 큐에 추가
	}
	// 2. 일반 경험치 획득 (Ratio가 1.0 미만)
	else
	{
		// 팁: 이미 1.0 미만의 일반 목표가 큐에 있다면 새로운 목표로 덮어씌워 애니메이션이 끊기지 않게 함
		if (TargetPercentQueue.Num() > 0 && TargetPercentQueue.Last() < 1.0f)
		{
			TargetPercentQueue.Last() = Ratio;
		}
		else
		{
			TargetPercentQueue.Add(Ratio);
		}
	}
	
}
