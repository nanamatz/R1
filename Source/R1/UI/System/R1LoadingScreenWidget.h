

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1LoadingScreenWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSceneLoadingFinished);
class UR1ProgressWidget;
/**
 * 
 */
UCLASS()
class R1_API UR1LoadingScreenWidget : public UR1UserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleVisualFinished();
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UR1ProgressWidget> WBP_ProgressBar;

public:
	// 서브시스템이 이 함수를 호출하여 데이터를 전달합니다.
	void SetProgress(float InPercent);
public:
	UPROPERTY(BlueprintAssignable)
	FOnSceneLoadingFinished OnSceneFinished;
};
