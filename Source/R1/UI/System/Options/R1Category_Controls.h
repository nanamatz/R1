
#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1Category_Controls.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraShakeChangedSignature, float, NewValue);

UCLASS()
class R1_API UR1Category_Controls : public UR1UserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnCameraShakeChangedSignature OnCameraShakeChanged;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1SettingRow_Slider> WBP_Slider_CameraShake;

private:
	UFUNCTION()
	void HandleCameraShakeChanged(float Value);
};
