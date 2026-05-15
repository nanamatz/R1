#include "UI/System/R1OptionsMenuWidget.h"
#include "System/R1SettingsSubsystem.h"
#include "System/R1SaveGame_Settings.h"

void UR1OptionsMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SyncUIFromSettings();
}

void UR1OptionsMenuWidget::SyncUIFromSettings()
{
    if (UR1SettingsSubsystem* SettingsSubsystem = GetGameInstance()->GetSubsystem<UR1SettingsSubsystem>())
    {
        if (UR1SaveGame_Settings* Settings = SettingsSubsystem->GetCustomSettings())
        {
            TempMasterVolume = Settings->MasterVolume;
            TempBGMVolume = Settings->BGMVolume;
            TempSFXVolume = Settings->SFXVolume;
            bTempShowDamageText = Settings->bShowDamageText;
            TempMinimapOpacity = Settings->MinimapOpacity;
            bTempConfineMouse = Settings->bConfineMouseToWindow;
            TempCameraShakeIntensity = Settings->CameraShakeIntensity;
        }
    }
}

void UR1OptionsMenuWidget::ApplyAndSaveSettings()
{
    if (UR1SettingsSubsystem* SettingsSubsystem = GetGameInstance()->GetSubsystem<UR1SettingsSubsystem>())
    {
        if (UR1SaveGame_Settings* Settings = SettingsSubsystem->GetCustomSettings())
        {
            Settings->MasterVolume = TempMasterVolume;
            Settings->BGMVolume = TempBGMVolume;
            Settings->SFXVolume = TempSFXVolume;
            Settings->bShowDamageText = bTempShowDamageText;
            Settings->MinimapOpacity = TempMinimapOpacity;
            Settings->bConfineMouseToWindow = bTempConfineMouse;
            Settings->CameraShakeIntensity = TempCameraShakeIntensity;

            SettingsSubsystem->SaveSettings();
        }
    }
}
