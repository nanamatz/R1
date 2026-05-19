

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1Category_Graphics.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResolutionSelectedSignature, int32, SelectedIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWindowModeSelectedSignature, int32, SelectedIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVSyncChangedSignature, bool, bIsEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFPSChangedSignature, float, NewFPS);

UCLASS()
class R1_API UR1Category_Graphics : public UR1UserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnResolutionSelectedSignature OnResolutionSelected;

	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnWindowModeSelectedSignature OnWindowModeSelected;

	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnVSyncChangedSignature OnVSyncChanged;

	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnFPSChangedSignature OnFPSChanged;

	UFUNCTION(BlueprintCallable, Category = "R1|UI|Graphics")
	void InitResolutions(const TArray<FString>& InResList);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UComboBoxString> ComboBox_Resolution;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UComboBoxString> ComboBox_WindowMode;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCheckBox> WBP_CheckBox_VSync;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1SettingRow_Slider> WBP_Slider_FPS;

private:
	UFUNCTION()
	void HandleResolutionSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleWindowModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleVSyncStateChanged(bool bIsChecked);

	UFUNCTION()
	void HandleFPSValueChanged(float Value);
};
