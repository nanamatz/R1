#include "UI/System/Options/R1SettingRow_Slider.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"

void UR1SettingRow_Slider::InitSlider(const FText& InOptionName, float MinValue, float MaxValue, float InitialValue)
{
    if (Text_OptionName)
    {
        Text_OptionName->SetText(InOptionName);
    }

    if (Slider_Value)
    {
        Slider_Value->SetMinValue(MinValue);
        Slider_Value->SetMaxValue(MaxValue);
        Slider_Value->SetValue(InitialValue);
    }

    UpdateDisplay(InitialValue);
}

void UR1SettingRow_Slider::SetValue(float NewValue)
{
    if (Slider_Value)
    {
        Slider_Value->SetValue(NewValue);
    }

    UpdateDisplay(NewValue);
}

void UR1SettingRow_Slider::SetOptionName(const FText& InOptionName)
{
    if (Text_OptionName)
    {
        Text_OptionName->SetText(InOptionName);
    }
}

void UR1SettingRow_Slider::NativeConstruct()
{
    Super::NativeConstruct();

    if (Slider_Value)
    {
        Slider_Value->OnValueChanged.AddDynamic(this, &UR1SettingRow_Slider::HandleInternalSliderValueChanged);
    }
}

void UR1SettingRow_Slider::HandleInternalSliderValueChanged(float NewValue)
{
    UpdateDisplay(NewValue);
    OnValueChanged.Broadcast(NewValue);
}

void UR1SettingRow_Slider::UpdateDisplay(float Value)
{
    if (Text_ValueDisplay)
    {
        float ScaledValue = Value * DisplayMultiplier;
        Text_ValueDisplay->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), ScaledValue)));
    }
}
