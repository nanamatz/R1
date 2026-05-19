
#include "UI/System/Options/R1Category_Audio.h"
#include "UI/System/Options/R1SettingRow_Slider.h"

void UR1Category_Audio::NativeConstruct()
{
	Super::NativeConstruct();

	if (WBP_Slider_Master)
	{
		WBP_Slider_Master->DisplayMultiplier = 100.0f;
		WBP_Slider_Master->InitSlider(FText::FromString(TEXT("Master Volume")), 0.0f, 1.0f, 1.0f);
		WBP_Slider_Master->OnValueChanged.AddDynamic(this, &UR1Category_Audio::HandleMasterVolumeChanged);
	}

	if (WBP_Slider_BGM)
	{
		WBP_Slider_BGM->DisplayMultiplier = 100.0f;
		WBP_Slider_BGM->InitSlider(FText::FromString(TEXT("Music")), 0.0f, 1.0f, 1.0f);
		WBP_Slider_BGM->OnValueChanged.AddDynamic(this, &UR1Category_Audio::HandleBGMVolumeChanged);
	}

	if (WBP_Slider_SFX)
	{
		WBP_Slider_SFX->DisplayMultiplier = 100.0f;
		WBP_Slider_SFX->InitSlider(FText::FromString(TEXT("SFX")), 0.0f, 1.0f, 1.0f);
		WBP_Slider_SFX->OnValueChanged.AddDynamic(this, &UR1Category_Audio::HandleSFXVolumeChanged);
	}
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
