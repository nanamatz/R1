


#include "UI/System/Options/R1Category_Graphics.h"
#include "Components/ComboBoxString.h"
#include "UI/System/Options/R1SettingRow_CheckBox.h"
#include "UI/System/Options/R1SettingRow_Slider.h"
#include "System/R1LocalizationSubsystem.h"


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
		WBP_CheckBox_VSync->InitCheckBox(FText::GetEmpty(), false);
		WBP_CheckBox_VSync->OnCheckStateChanged.AddDynamic(this, &UR1Category_Graphics::HandleVSyncStateChanged);
	}

	if (WBP_Slider_FPS)
	{
		WBP_Slider_FPS->InitSlider(FText::GetEmpty(), 30.0f, 144.0f, 60.0f);
		WBP_Slider_FPS->OnValueChanged.AddDynamic(this, &UR1Category_Graphics::HandleFPSValueChanged);
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.AddUObject(this, &UR1Category_Graphics::RefreshLocalization);
		}
	}
	RefreshLocalization();
}

void UR1Category_Graphics::NativeDestruct()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.RemoveAll(this);
		}
	}
	Super::NativeDestruct();
}

void UR1Category_Graphics::RefreshLocalization()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>();
	if (!LocSub) return;

	if (WBP_CheckBox_VSync) WBP_CheckBox_VSync->SetOptionName(LocSub->GetText(TEXT("Option_VSync")));
	if (WBP_Slider_FPS)     WBP_Slider_FPS->SetOptionName(LocSub->GetText(TEXT("Option_MaxFrameRate")));
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

void UR1Category_Graphics::SetSelectedIndexes(int32 ResIndex, int32 WindowModeIndex)
{
	if (ComboBox_Resolution)
	{
		ComboBox_Resolution->SetSelectedIndex(ResIndex);
	}

	if (ComboBox_WindowMode)
	{
		ComboBox_WindowMode->SetSelectedIndex(WindowModeIndex);
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
