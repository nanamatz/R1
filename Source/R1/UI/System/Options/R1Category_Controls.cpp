
#include "UI/System/Options/R1Category_Controls.h"
#include "UI/System/Options/R1SettingRow_Slider.h"

void UR1Category_Controls::NativeConstruct()
{
	Super::NativeConstruct();

	if (WBP_Slider_CameraShake)
	{
		WBP_Slider_CameraShake->InitSlider(FText::FromString(TEXT("화면 흔들림 강도")), 0.0f, 1.0f, 1.0f);
		WBP_Slider_CameraShake->OnValueChanged.AddDynamic(this, &UR1Category_Controls::HandleCameraShakeChanged);
	}
}

void UR1Category_Controls::HandleCameraShakeChanged(float Value)
{
	if (OnCameraShakeChanged.IsBound())
	{
		OnCameraShakeChanged.Broadcast(Value);
	}
}
