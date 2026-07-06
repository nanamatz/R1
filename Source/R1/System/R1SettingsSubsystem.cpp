#include "System/R1SettingsSubsystem.h"
#include "R1LogChannels.h"
#include "System/R1SaveGame_Settings.h"
#include "System/R1LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

void UR1SettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Collection.InitializeDependency<UR1LocalizationSubsystem>();
    Super::Initialize(Collection);

    MasterSoundMix = Cast<USoundMix>(StaticLoadObject(USoundMix::StaticClass(), nullptr, TEXT("/Script/Engine.SoundMix'/Game/Blueprints/Audio/SM_Main.SM_Main'")));
    MasterSoundClass = Cast<USoundClass>(StaticLoadObject(USoundClass::StaticClass(), nullptr, TEXT("/Script/Engine.SoundClass'/Game/Blueprints/Audio/SC_Master.SC_Master'")));
    BGMSoundClass = Cast<USoundClass>(StaticLoadObject(USoundClass::StaticClass(), nullptr, TEXT("/Script/Engine.SoundClass'/Game/Blueprints/Audio/SC_BGM.SC_BGM'")));
    SFXSoundClass = Cast<USoundClass>(StaticLoadObject(USoundClass::StaticClass(), nullptr, TEXT("/Script/Engine.SoundClass'/Game/Blueprints/Audio/SC_SFX.SC_SFX'")));

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
            UE_LOG(LogR1, Warning, TEXT("[R1Settings] Applying Graphics: Res %dx%d, Mode %d, FPS %f, VSync %d"), 
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
    if (CurrentSettings)
    {
        // 월드가 현재 존재하는지(레벨 이동 중이 아닌지) 안전 검사
        if (UWorld* World = GetWorld())
        {
            if (MasterSoundMix)
            {
                if (MasterSoundClass)
                {
                    UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMix, MasterSoundClass, CurrentSettings->MasterVolume, 1.0f, 0.0f, true);
                }
                if (BGMSoundClass)
                {
                    UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMix, BGMSoundClass, CurrentSettings->BGMVolume, 1.0f, 0.0f, true);
                }
                if (SFXSoundClass)
                {
                    UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMix, SFXSoundClass, CurrentSettings->SFXVolume, 1.0f, 0.0f, true);
                }

                // 변경된 믹스 세팅을 엔진에 푸시(적용)
                UGameplayStatics::PushSoundMixModifier(World, MasterSoundMix);
            }
        }
    }

    // UI 동기화 등을 위한 브로드캐스트
    if (OnAudioSettingsChanged.IsBound())
    {
        OnAudioSettingsChanged.Broadcast();
    }
}

void UR1SettingsSubsystem::ApplyGameplaySettings()
{
    if (CurrentSettings)
    {
        if (UGameInstance* GI = GetGameInstance())
        {
            if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
            {
                LocSub->SetLanguage(CurrentSettings->Language);
            }
        }
    }

    if (OnGameplaySettingsChanged.IsBound())
    {
        OnGameplaySettingsChanged.Broadcast();
    }
}

void UR1SettingsSubsystem::ApplyControlSettings()
{
    if (!CurrentSettings) return;

    // 마우스 커서 가두기 설정 적용
    if (UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient())
    {
        EMouseLockMode LockMode = CurrentSettings->bConfineMouseToWindow ? EMouseLockMode::LockAlways : EMouseLockMode::DoNotLock;
        ViewportClient->SetMouseLockMode(LockMode);
    }
}
