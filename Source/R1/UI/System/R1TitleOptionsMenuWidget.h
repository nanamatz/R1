
#pragma once

#include "CoreMinimal.h"
#include "UI/System/R1OptionsMenuWidget.h"
#include "R1TitleOptionsMenuWidget.generated.h"

/**
 * 타이틀 씬(아웃게임) 전용 옵션 위젯
 */
UCLASS()
class R1_API UR1TitleOptionsMenuWidget : public UR1OptionsMenuWidget
{
	GENERATED_BODY()

public:
	// 타이틀용 특수 로직이 필요할 경우 여기에 추가 (현재는 구조 분리가 목적)
};
