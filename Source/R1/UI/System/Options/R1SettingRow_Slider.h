#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1SettingRow_Slider.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSliderRowValueChangedSignature, float, NewValue);

UCLASS()
class R1_API UR1SettingRow_Slider : public UR1UserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable, Category = "R1|Events")
    FOnSliderRowValueChangedSignature OnValueChanged;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "R1|UI")
    float DisplayMultiplier = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void InitSlider(const FText& InOptionName, float MinValue, float MaxValue, float InitialValue);

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void SetValue(float NewValue);

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void SetOptionName(const FText& InOptionName);

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> Text_OptionName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> Text_ValueDisplay;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class USlider> Slider_Value;

private:
    UFUNCTION()
    void HandleInternalSliderValueChanged(float NewValue);

    void UpdateDisplay(float Value);
};
