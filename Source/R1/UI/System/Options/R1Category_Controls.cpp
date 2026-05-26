
#include "UI/System/Options/R1Category_Controls.h"
#include "UI/System/Options/R1SettingRow_Slider.h"
#include "UI/System/Options/R1SettingRow_CheckBox.h"
#include "System/R1LocalizationSubsystem.h"

void UR1Category_Controls::NativeConstruct()
{
	Super::NativeConstruct();

	if (WBP_Slider_CameraShake)
	{
		WBP_Slider_CameraShake->DisplayMultiplier = 100.0f;
		WBP_Slider_CameraShake->InitSlider(FText::GetEmpty(), 0.0f, 1.0f, 1.0f);
		WBP_Slider_CameraShake->OnValueChanged.AddDynamic(this, &UR1Category_Controls::HandleCameraShakeChanged);
	}

	if (WBP_CheckBox_ConfineMouse)
	{
		WBP_CheckBox_ConfineMouse->InitCheckBox(FText::GetEmpty(), true);
		WBP_CheckBox_ConfineMouse->OnCheckStateChanged.AddDynamic(this, &UR1Category_Controls::HandleConfineMouseChanged);
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.AddUObject(this, &UR1Category_Controls::RefreshLocalization);
		}
	}
	RefreshLocalization();
}

void UR1Category_Controls::NativeDestruct()
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

void UR1Category_Controls::RefreshLocalization()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>();
	if (!LocSub) return;

	if (WBP_Slider_CameraShake)    WBP_Slider_CameraShake->SetOptionName(LocSub->GetText(TEXT("Option_CameraShake")));
	if (WBP_CheckBox_ConfineMouse) WBP_CheckBox_ConfineMouse->SetOptionName(LocSub->GetText(TEXT("Option_ClipCursor")));
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
