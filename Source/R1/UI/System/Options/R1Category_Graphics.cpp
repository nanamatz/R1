


#include "UI/System/Options/R1Category_Graphics.h"
#include "Components/ComboBoxString.h"
#include "UI/System/Options/R1SettingRow_CheckBox.h"
#include "UI/System/Options/R1SettingRow_Slider.h"


void UR1Category_Graphics::NativeConstruct()
{
	Super::NativeConstruct();

	if (ComboBox_Resolution)
	{
		ComboBox_Resolution->OnSelectionChanged.AddDynamic(this, &UR1Category_Graphics::HandleResolutionSelectionChanged);
	}

	if (ComboBox_WindowMode)
	{
		ComboBox_WindowMode->OnSelectionChanged.AddDynamic(this, &UR1Category_Graphics::HandleWindowModeSelectionChanged);
	}

	if (WBP_CheckBox_VSync)
	{
		WBP_CheckBox_VSync->InitCheckBox(FText::FromString(TEXT("VSync")), false);
		WBP_CheckBox_VSync->OnCheckStateChanged.AddDynamic(this, &UR1Category_Graphics::HandleVSyncStateChanged);
	}

	if (WBP_Slider_FPS)
	{
		WBP_Slider_FPS->InitSlider(FText::FromString(TEXT("Max Frame Rate")), 30.0f, 144.0f, 60.0f);
		WBP_Slider_FPS->OnValueChanged.AddDynamic(this, &UR1Category_Graphics::HandleFPSValueChanged);
	}
}

void UR1Category_Graphics::InitResolutions(const TArray<FString>& InResList)
{
	if (!ComboBox_Resolution) return;

	ComboBox_Resolution->ClearOptions();

	for (const FString& ResString : InResList)
	{
		ComboBox_Resolution->AddOption(ResString);
	}
}

void UR1Category_Graphics::HandleResolutionSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!ComboBox_Resolution) return;

	int32 SelectedIndex = ComboBox_Resolution->FindOptionIndex(SelectedItem);

	if (OnResolutionSelected.IsBound())
	{
		OnResolutionSelected.Broadcast(SelectedIndex);
	}
}

void UR1Category_Graphics::HandleWindowModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!ComboBox_WindowMode) return;

	int32 SelectedIndex = ComboBox_WindowMode->FindOptionIndex(SelectedItem);

	if (OnWindowModeSelected.IsBound())
	{
		OnWindowModeSelected.Broadcast(SelectedIndex);
	}
}

void UR1Category_Graphics::HandleVSyncStateChanged(bool bIsChecked)
{
	if (OnVSyncChanged.IsBound())
	{
		OnVSyncChanged.Broadcast(bIsChecked);
	}
}

void UR1Category_Graphics::HandleFPSValueChanged(float Value)
{
	if (OnFPSChanged.IsBound())
	{
		OnFPSChanged.Broadcast(Value);
	}
}
