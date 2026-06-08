


#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1PlayerInfoWidget.generated.h"

/**
 * HP/MP 오브 등을 담는 플레이어 정보 UI. HP 오브 위에 덮인 투명 버튼(히트박스)의
 * 호버를 감지해 오브의 HP 텍스트 오버레이를 토글한다. (스킬 프레임이 오브를 덮어도 동작)
 */
UCLASS()
class R1_API UR1PlayerInfoWidget : public UR1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnHpOrbHitBoxHovered();

	UFUNCTION()
	void OnHpOrbHitBoxUnhovered();

	UFUNCTION()
	void OnMpOrbHitBoxHovered();

	UFUNCTION()
	void OnMpOrbHitBoxUnhovered();

private:
	// 이 위젯 안의 HP 오브 위젯 (이름 일치 필수: HpOrbWidget)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UR1HpOrbWidget> HpOrbWidget;

	// 오브 위(스킬 프레임보다 앞)에 덮은 투명 버튼. 호버 감지용 (이름 일치 필수: Btn_HpOrbHover)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UButton> Btn_HpOrbHover;

	// 이 위젯 안의 MP 오브 위젯 (이름 일치 필수: MpOrbWidget)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UR1MpOrbWidget> MpOrbWidget;

	// MP 오브 위에 덮은 투명 버튼. 호버 감지용 (이름 일치 필수: Btn_MpOrbHover)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UButton> Btn_MpOrbHover;
};
