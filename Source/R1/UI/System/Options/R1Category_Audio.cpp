
#include "UI/System/Options/R1Category_Audio.h"
#include "UI/System/Options/R1SettingRow_Slider.h"

void UR1Category_Audio::NativeConstruct()
{
	Super::NativeConstruct();

	if (WBP_Slider_Master)
	{
		WBP_Slider_Master->InitSlider(FText::FromString(TEXT("마스터 볼륨")), 0.0f, 1.0f, 1.0f);
		WBP_Slider_Master->OnValueChanged.AddDynamic(this, &UR1Category_Audio::HandleMasterVolumeChanged);
	}

	if (WBP_Slider_BGM)
	{
		WBP_Slider_BGM->InitSlider(FText::FromString(TEXT("배경 음악")), 0.0f, 1.0f, 1.0f);
		WBP_Slider_BGM->OnValueChanged.AddDynamic(this, &UR1Category_Audio::HandleBGMVolumeChanged);
	}

	if (WBP_Slider_SFX)
	{
		WBP_Slider_SFX->InitSlider(FText::FromString(TEXT("효과음")), 0.0f, 1.0f, 1.0f);
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
