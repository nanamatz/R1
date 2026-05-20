
#include "UI/System/R1GameOptionsMenuWidget.h"

void UR1GameOptionsMenuWidget::OnApplyButtonClicked()
{
	// 1. 부모의 기본 적용 로직 실행 (엔진 상태 업데이트)
	Super::OnApplyButtonClicked();

	// 2. 인게임에서는 적용 후 즉시 메뉴를 닫고 게임메뉴로 돌아가도록 함
	OnCloseRequested.Broadcast();
}
