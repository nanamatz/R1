
#include "UI/System/Options/R1Category_Controls.h"
#include "UI/System/Options/R1SettingRow_Slider.h"
#include "UI/System/Options/R1SettingRow_CheckBox.h"

void UR1Category_Controls::NativeConstruct()
{
	Super::NativeConstruct();

	if (WBP_Slider_CameraShake)
	{
		WBP_Slider_CameraShake->DisplayMultiplier = 100.0f;
		WBP_Slider_CameraShake->InitSlider(FText::FromString(TEXT("Camera Shake Intensity")), 0.0f, 1.0f, 1.0f);
		WBP_Slider_CameraShake->OnValueChanged.AddDynamic(this, &UR1Category_Controls::HandleCameraShakeChanged);
	}

	if (WBP_CheckBox_ConfineMouse)
	{
		WBP_CheckBox_ConfineMouse->InitCheckBox(FText::FromString(TEXT("Clip Cursor")), true);
		WBP_CheckBox_ConfineMouse->OnCheckStateChanged.AddDynamic(this, &UR1Category_Controls::HandleConfineMouseChanged);
	}
}

void UR1Category_Controls::HandleCameraShakeChanged(float Value)
{
	if (OnCameraShakeChanged.IsBound())
	{
		OnCameraShakeChanged.Broadcast(Value);
	}
}

void UR1Category_Controls::HandleConfineMouseChanged(bool bIsChecked)
{
	if (OnConfineMouseChanged.IsBound())
	{
		OnConfineMouseChanged.Broadcast(bIsChecked);
	}
}
