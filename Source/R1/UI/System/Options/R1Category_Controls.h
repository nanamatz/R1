
#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1Category_Controls.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraShakeChangedSignature, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConfineMouseChangedSignature, bool, bIsEnabled);

UCLASS()
class R1_API UR1Category_Controls : public UR1UserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnCameraShakeChangedSignature OnCameraShakeChanged;

	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnConfineMouseChangedSignature OnConfineMouseChanged;

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1SettingRow_Slider> WBP_Slider_CameraShake;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1SettingRow_CheckBox> WBP_CheckBox_ConfineMouse;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleCameraShakeChanged(float Value);

	UFUNCTION()
	void HandleConfineMouseChanged(bool bIsChecked);
};
