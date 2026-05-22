
#include "UI/System/Options/R1Category_Graphics.h"
#include "UI/System/Options/R1SettingRow_ComboBox.h"
#include "UI/System/Options/R1SettingRow_CheckBox.h"
#include "UI/System/Options/R1SettingRow_Slider.h"
#include "System/R1LocalizationSubsystem.h"


void UR1Category_Graphics::NativeConstruct()
{
	Super::NativeConstruct();

	if (WBP_ComboBox_Resolution)
	{
		WBP_ComboBox_Resolution->OnSelectionChanged.AddDynamic(this, &UR1Category_Graphics::HandleResolutionSelectionChanged);
	}

	if (WBP_ComboBox_WindowMode)
	{
		WBP_ComboBox_WindowMode->OnSelectionChanged.AddDynamic(this, &UR1Category_Graphics::HandleWindowModeSelectionChanged);
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

	if (WBP_ComboBox_Resolution) WBP_ComboBox_Resolution->SetOptionName(LocSub->GetText(TEXT("Option_Resolution")));
	if (WBP_ComboBox_WindowMode) WBP_ComboBox_WindowMode->SetOptionName(LocSub->GetText(TEXT("Option_WindowMode")));
	if (WBP_CheckBox_VSync)      WBP_CheckBox_VSync->SetOptionName(LocSub->GetText(TEXT("Option_VSync")));
	if (WBP_Slider_FPS)          WBP_Slider_FPS->SetOptionName(LocSub->GetText(TEXT("Option_MaxFrameRate")));
}

void UR1Category_Graphics::InitResolutions(const TArray<FString>& InResList)
{
	if (WBP_ComboBox_Resolution)
	{
		WBP_ComboBox_Resolution->SetOptions(InResList);
	}
}

void UR1Category_Graphics::InitWindowModes(const TArray<FString>& InModeList)
{
	if (WBP_ComboBox_WindowMode)
	{
		WBP_ComboBox_WindowMode->SetOptions(InModeList);
	}
}

void UR1Category_Graphics::SetSelectedIndexes(int32 ResIndex, int32 WindowModeIndex)
{
	if (WBP_ComboBox_Resolution)
	{
		WBP_ComboBox_Resolution->SetSelectedIndex(ResIndex);
	}

	if (WBP_ComboBox_WindowMode)
	{
		WBP_ComboBox_WindowMode->SetSelectedIndex(WindowModeIndex);
	}
}

void UR1Category_Graphics::HandleResolutionSelectionChanged(int32 SelectedIndex)
{
	OnResolutionSelected.Broadcast(SelectedIndex);
}

void UR1Category_Graphics::HandleWindowModeSelectionChanged(int32 SelectedIndex)
{
	OnWindowModeSelected.Broadcast(SelectedIndex);
}

void UR1Category_Graphics::HandleVSyncStateChanged(bool bIsChecked)
{
	OnVSyncChanged.Broadcast(bIsChecked);
}

void UR1Category_Graphics::HandleFPSValueChanged(float Value)
{
	OnFPSChanged.Broadcast(Value);
}
