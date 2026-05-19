


#include "UI/System/Options/R1Category_Graphics.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"


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

	if (CheckBox_VSync)
	{
		CheckBox_VSync->OnCheckStateChanged.AddDynamic(this, &UR1Category_Graphics::HandleVSyncStateChanged);
	}

	if (Slider_FPS)
	{
		Slider_FPS->OnValueChanged.AddDynamic(this, &UR1Category_Graphics::HandleFPSValueChanged);
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
