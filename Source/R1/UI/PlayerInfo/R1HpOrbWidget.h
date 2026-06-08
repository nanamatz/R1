

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "R1HpOrbWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1HpOrbWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
public:
	// 캐릭터의 델리게이트가 호출할 함수
	UFUNCTION()
	void UpdateHpOrb(float Ratio);

	// HP 텍스트 오버레이 표시 토글 (호버 히트박스 등 외부에서 호출)
	void SetHpTextVisible(bool bVisible);

protected:
	// 1. UMG에 배치된 이미지 (이름 일치 필수: HpOrbImage)
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	TObjectPtr<class UImage> HpOrb;

	// WBP_HpOrb 안에 "HpTextUI" 이름으로 배치된 자식 위젯 (이름 일치 필수). 호버 시 표시.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1HpTextUI> HpTextUI;

	// 2. 머티리얼 파라미터 이름 (에디터에서 "Percent" 등으로 설정한 이름)
	UPROPERTY(EditAnywhere, Category = "Material")
	FName MaterialParamName = FName("Percentage");

private:
	// 3. 런타임에 제어할 동적 머티리얼 인스턴스
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> DynamicOrbMaterial;
	
	//이벤트 발생 시 이 값을 변경
	float TargetPercent = 1.0f;
};
