
#include "UI/System/Options/R1Category_Audio.h"
#include "UI/System/Options/R1SettingRow_Slider.h"
#include "System/R1LocalizationSubsystem.h"

void UR1Category_Audio::NativeConstruct()
{
	Super::NativeConstruct();

	if (WBP_Slider_Master)
	{
		WBP_Slider_Master->DisplayMultiplier = 100.0f;
		WBP_Slider_Master->InitSlider(FText::GetEmpty(), 0.0f, 1.0f, 1.0f);
		WBP_Slider_Master->OnValueChanged.AddDynamic(this, &UR1Category_Audio::HandleMasterVolumeChanged);
	}

	if (WBP_Slider_BGM)
	{
		WBP_Slider_BGM->DisplayMultiplier = 100.0f;
		WBP_Slider_BGM->InitSlider(FText::GetEmpty(), 0.0f, 1.0f, 1.0f);
		WBP_Slider_BGM->OnValueChanged.AddDynamic(this, &UR1Category_Audio::HandleBGMVolumeChanged);
	}

	if (WBP_Slider_SFX)
	{
		WBP_Slider_SFX->DisplayMultiplier = 100.0f;
		WBP_Slider_SFX->InitSlider(FText::GetEmpty(), 0.0f, 1.0f, 1.0f);
		WBP_Slider_SFX->OnValueChanged.AddDynamic(this, &UR1Category_Audio::HandleSFXVolumeChanged);
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.AddUObject(this, &UR1Category_Audio::RefreshLocalization);
		}
	}
	RefreshLocalization();
}

void UR1Category_Audio::NativeDestruct()
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

void UR1Category_Audio::RefreshLocalization()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>();
	if (!LocSub) return;

	if (WBP_Slider_Master) WBP_Slider_Master->SetOptionName(LocSub->GetText(TEXT("Option_MasterVolume")));
	if (WBP_Slider_BGM)    WBP_Slider_BGM->SetOptionName(LocSub->GetText(TEXT("Option_Music")));
	if (WBP_Slider_SFX)    WBP_Slider_SFX->SetOptionName(LocSub->GetText(TEXT("Option_SFX")));
}

void UR1Category_Audio::HandleMasterVolumeChanged(float Value)
{
	if (OnMasterVolumeChanged.IsBound())
	{
		OnMasterVolumeChanged.Broadcast(Value);
	}
}

void UR1Category_Audio::HandleBGMVolumeChanged(float Value)
{
	if (OnBGMVolumeChanged.IsBound())
	{
		OnBGMVolumeChanged.Broadcast(Value);
	}
}

void UR1Category_Audio::HandleSFXVolumeChanged(float Value)
{
	if (OnSFXVolumeChanged.IsBound())
	{
		OnSFXVolumeChanged.Broadcast(Value);
	}
}
