


#include "UI/System/R1FloorGuideSceneWidget.h"
#include "UI/System/R1FloorGuideWidget.h"

void UR1FloorGuideSceneWidget::ShowFloorGuide(ER1FloorLevel FloorLevel)
{
	// 1. 자기 자신을 화면에 보이게 켭니다.
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// 2. 내부 알맹이 위젯에게 Enum 데이터를 넘겨 애니메이션을 틀게 합니다.
	if (WBP_FloorGuide)
	{
		WBP_FloorGuide->PlayAnnouncement(FloorLevel);
	}

	// 3. 3.5초 뒤에 스스로를 다시 숨깁니다! (애니메이션 길이에 맞춰 시간 조절)
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UR1FloorGuideSceneWidget::HideScene, 3.f, false);
}

void UR1FloorGuideSceneWidget::HideScene()
{
	SetVisibility(ESlateVisibility::Hidden);
}
