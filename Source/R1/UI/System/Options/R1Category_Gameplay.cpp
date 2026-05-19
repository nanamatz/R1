
#include "UI/System/Options/R1Category_Gameplay.h"
#include "UI/System/Options/R1SettingRow_Slider.h"
#include "UI/System/Options/R1SettingRow_CheckBox.h"

void UR1Category_Gameplay::NativeConstruct()
{
	Super::NativeConstruct();

	if (WBP_Slider_MinimapOpacity)
	{
		WBP_Slider_MinimapOpacity->DisplayMultiplier = 100.0f;
		WBP_Slider_MinimapOpacity->InitSlider(FText::FromString(TEXT("Minimap Opacity")), 0.1f, 1.0f, 0.5f);
		WBP_Slider_MinimapOpacity->OnValueChanged.AddDynamic(this, &UR1Category_Gameplay::HandleMinimapOpacityChanged);
	}

	if (WBP_CheckBox_ShowDamageText)
	{
		WBP_CheckBox_ShowDamageText->InitCheckBox(FText::FromString(TEXT("Floating Damage")), true);
		WBP_CheckBox_ShowDamageText->OnCheckStateChanged.AddDynamic(this, &UR1Category_Gameplay::HandleShowDamageTextChanged);
	}
}

void UR1Category_Gameplay::HandleMinimapOpacityChanged(float Value)
{
	if (OnMinimapOpacityChanged.IsBound())
	{
		OnMinimapOpacityChanged.Broadcast(Value);
	}
}

void UR1Category_Gameplay::HandleShowDamageTextChanged(bool bIsChecked)
{
	if (OnShowDamageTextChanged.IsBound())
	{
		OnShowDamageTextChanged.Broadcast(bIsChecked);
	}
}
