
#pragma once

#include "CoreMinimal.h"
#include "UI/System/R1OptionsMenuWidget.h"
#include "R1GameOptionsMenuWidget.generated.h"

/**
 * 인게임(런 상태) 전용 옵션 위젯
 */
UCLASS()
class R1_API UR1GameOptionsMenuWidget : public UR1OptionsMenuWidget
{
	GENERATED_BODY()

public:
	// 인게임에서는 적용(Apply) 버튼도 창을 닫고 복귀하도록 설정 (사용자 요청 사항)
	virtual void OnApplyButtonClicked() override;

	// 확인(Confirm)과 취소(Cancel)는 부모 로직을 따르되, 
	// 부모에서 이미 OnCloseRequested.Broadcast()를 호출하므로 별도 오버라이드 없이도 작동함.
	// 만약 인게임만의 특수한 후처리가 필요하다면 여기서 수행.
};
