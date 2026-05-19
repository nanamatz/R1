
#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1Category_Gameplay.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinimapOpacityChangedSignature, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShowDamageTextChangedSignature, bool, bIsEnabled);

UCLASS()
class R1_API UR1Category_Gameplay : public UR1UserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnMinimapOpacityChangedSignature OnMinimapOpacityChanged;

	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnShowDamageTextChangedSignature OnShowDamageTextChanged;

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1SettingRow_Slider> WBP_Slider_MinimapOpacity;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1SettingRow_CheckBox> WBP_CheckBox_ShowDamageText;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleMinimapOpacityChanged(float Value);

	UFUNCTION()
	void HandleShowDamageTextChanged(bool bIsChecked);
};
