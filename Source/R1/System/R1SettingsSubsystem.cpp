#include "System/R1SettingsSubsystem.h"
#include "System/R1SaveGame_Settings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"

void UR1SettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadSettings();
}

void UR1SettingsSubsystem::ApplySettings()
{
    ApplyGraphicsSettings();
    ApplyAudioSettings();
    ApplyGameplaySettings();
    ApplyControlSettings();
}

void UR1SettingsSubsystem::SaveSettings()
{
    if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
    {
        UserSettings->SaveSettings();
    }

    if (CurrentSettings)
    {
        UGameplayStatics::SaveGameToSlot(CurrentSettings, SettingsSaveSlotName, SettingsUserIndex);
    }

    ApplySettings();
}

void UR1SettingsSubsystem::LoadSettings()
{
    if (UGameplayStatics::DoesSaveGameExist(SettingsSaveSlotName, SettingsUserIndex))
    {
        CurrentSettings = Cast<UR1SaveGame_Settings>(UGameplayStatics::LoadGameFromSlot(SettingsSaveSlotName, SettingsUserIndex));
    }

    if (!CurrentSettings)
    {
        CurrentSettings = Cast<UR1SaveGame_Settings>(UGameplayStatics::CreateSaveGameObject(UR1SaveGame_Settings::StaticClass()));
    }

    ApplySettings();
}

void UR1SettingsSubsystem::ApplyGraphicsSettings()
{
    if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
    {
        if (CurrentSettings)
        {
            UE_LOG(LogTemp, Warning, TEXT("[R1Settings] Applying Graphics: Res %dx%d, Mode %d, FPS %f, VSync %d"), 
                CurrentSettings->Resolution.X, CurrentSettings->Resolution.Y, 
                (int32)CurrentSettings->WindowMode.GetValue(), 
                CurrentSettings->FrameRateLimit, 
                CurrentSettings->bVSyncEnabled);

            UserSettings->SetScreenResolution(CurrentSettings->Resolution);
            UserSettings->SetFullscreenMode(CurrentSettings->WindowMode);
            UserSettings->SetFrameRateLimit(CurrentSettings->FrameRateLimit);
            UserSettings->SetVSyncEnabled(CurrentSettings->bVSyncEnabled);
        }

        // false: Standalone 모드 실행 시 에디터가 넘겨주는 명령행 인자(-windowed 등)를 무시하고 
        // 우리가 설정한 값을 강제로 적용합니다.
        UserSettings->ApplySettings(false); 
    }
}

void UR1SettingsSubsystem::ApplyAudioSettings()
{
    // TODO: Implement audio volume setting logic (e.g., via SoundMix or AudioDevice)
}

void UR1SettingsSubsystem::ApplyGameplaySettings()
{
    // TODO: Implement gameplay setting application logic
}

void UR1SettingsSubsystem::ApplyControlSettings()
{
    // TODO: Implement control/accessibility application logic
}
