

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1ProgressWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVisualLoadingFinished);
class UImage;
class UTextBlock;
class UMaterialInstanceDynamic;
/**
 * 
 */
UCLASS()
class R1_API UR1ProgressWidget : public UR1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Ring;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Percent;
	// 실시간으로 조작할 머티리얼을 담아둘 포인터
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ProgressMaterial;

	float CurrentVisualProgress;
	float TargetVisualProgress;

public:
	// 외부(맵 제너레이터 등)에서 로딩 퍼센트를 넘겨줄 때 사용할 함수
	UFUNCTION(BlueprintCallable)
	void UpdateProgress(float InPercent);

	UPROPERTY(BlueprintAssignable)
	FOnVisualLoadingFinished OnFinished;

private:
	// 방송 중복 실행 방지용 플래그
	bool bHasFiredFinished = false;
};
