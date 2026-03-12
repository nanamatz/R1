

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1Define.h"
#include "R1FloorGuideWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;
/**
 * 
 */
UCLASS()
class R1_API UR1FloorGuideWidget : public UR1UserWidget
{
	GENERATED_BODY()

protected:
	// UMG에 배치된 텍스트 변수 (이름이 똑같아야 합니다)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_FloorName;

	// UMG에서 만든 페이드 인/아웃 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> FadeInOutAnimation;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> AppearingAnimation;

public:
	// Enum 값을 받아서 텍스트로 바꾸고 애니메이션을 재생하는 함수
	void PlayAnnouncement(ER1FloorLevel FloorLevel);

private:
	// Enum을 예쁜 텍스트로 바꿔주는 헬퍼 함수
	FText GetFloorText(ER1FloorLevel FloorLevel);
};
