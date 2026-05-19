#include "UI/System/R1OptionsMenuWidget.h"
#include "System/R1SettingsSubsystem.h"
#include "System/R1SaveGame_Settings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/GameUserSettings.h"
#include "UI/System/Options/R1Category_Graphics.h"
#include "UI/System/Options/R1Category_Audio.h"
#include "UI/System/Options/R1Category_Gameplay.h"
#include "UI/System/Options/R1Category_Controls.h"
#include "UI/System/Options/R1SettingRow_Slider.h"
#include "UI/System/Options/R1SettingRow_CheckBox.h"
#include "UI/R1HUD.h"
#include "Components/Button.h"
#include "Player/R1MainMenuController.h"

void UR1OptionsMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SyncUIFromSettings();

    if (WBP_Category_Graphics)
    {
        WBP_Category_Graphics->InitResolutions(GenerateResolutionList());

        WBP_Category_Graphics->OnResolutionSelected.AddDynamic(this, &UR1OptionsMenuWidget::SetTempResolutionByIndex);
        WBP_Category_Graphics->OnWindowModeSelected.AddDynamic(this, &UR1OptionsMenuWidget::SetTempWindowModeByIndex);
        WBP_Category_Graphics->OnVSyncChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempVSync);
        WBP_Category_Graphics->OnFPSChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempFPS);
    }

    if (WBP_Category_Audio)
    {
        WBP_Category_Audio->OnMasterVolumeChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempMasterVolume);
        WBP_Category_Audio->OnBGMVolumeChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempBGMVolume);
        WBP_Category_Audio->OnSFXVolumeChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempSFXVolume);
    }

    if (WBP_Category_Gameplay)
    {
        WBP_Category_Gameplay->OnMinimapOpacityChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempMinimapOpacity);
        WBP_Category_Gameplay->OnShowDamageTextChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempShowDamageText);
    }

    if (WBP_Category_Controls)
    {
        WBP_Category_Controls->OnCameraShakeChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempCameraShakeIntensity);
        WBP_Category_Controls->OnConfineMouseChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempConfineMouse);
    }

    if (Button_Apply)
    {
        Button_Apply->OnClicked.AddDynamic(this, &UR1OptionsMenuWidget::OnApplyButtonClicked);
    }

    if (Button_Close)
    {
        Button_Close->OnClicked.AddDynamic(this, &UR1OptionsMenuWidget::OnCloseButtonClicked);
    }
}


TArray<FString> UR1OptionsMenuWidget::GenerateResolutionList()
{
    TArray<FString> StringList;
    SupportedResolutions.Empty();

    // 1. 현재 모니터가 지원하는 전체화면 해상도 목록 가져오기
    UKismetSystemLibrary::GetSupportedFullscreenResolutions(SupportedResolutions);

    // 2. 만약 엔진에서 목록을 가져오지 못했다면 안전을 위한 기본값 세팅
    if (SupportedResolutions.Num() == 0)
    {
        SupportedResolutions.Add(FIntPoint(1920, 1080));
        SupportedResolutions.Add(FIntPoint(2560, 1440));
    }

    // 3. UI 콤보박스에 표시할 "1920 x 1080" 형태의 문자열로 변환
    for (const FIntPoint& Res : SupportedResolutions)
    {
        FString ResString = FString::Printf(TEXT("%d x %d"), Res.X, Res.Y);
        StringList.Add(ResString);
    }

    return StringList;
}

void UR1OptionsMenuWidget::SetTempResolutionByIndex(int32 SelectedIndex)
{
    // 유효한 인덱스인지 안전 검사
    if (SupportedResolutions.IsValidIndex(SelectedIndex))
    {
        TempResolution = SupportedResolutions[SelectedIndex];
    }
}

void UR1OptionsMenuWidget::SetTempWindowModeByIndex(int32 SelectedIndex)
{
    switch (SelectedIndex)
    {
    case 0:
        TempWindowMode = EWindowMode::Fullscreen;
        break;
    case 1:
        TempWindowMode = EWindowMode::WindowedFullscreen;
        break;
    case 2:
        TempWindowMode = EWindowMode::Windowed;
        break;
    default:
        break;
    }
}

void UR1OptionsMenuWidget::SetTempVSync(bool bEnabled)
{
    bTempVSyncEnabled = bEnabled;
}

void UR1OptionsMenuWidget::SetTempFPS(float NewFPS)
{
    TempFrameRateLimit = NewFPS;
}

void UR1OptionsMenuWidget::SetTempMasterVolume(float NewVolume)
{
    TempMasterVolume = NewVolume;
}

void UR1OptionsMenuWidget::SetTempBGMVolume(float NewVolume)
{
    TempBGMVolume = NewVolume;
}

void UR1OptionsMenuWidget::SetTempSFXVolume(float NewVolume)
{
    TempSFXVolume = NewVolume;
}

void UR1OptionsMenuWidget::SetTempMinimapOpacity(float NewOpacity)
{
    TempMinimapOpacity = NewOpacity;
}

void UR1OptionsMenuWidget::SetTempShowDamageText(bool bEnabled)
{
    bTempShowDamageText = bEnabled;
}

void UR1OptionsMenuWidget::SetTempCameraShakeIntensity(float NewIntensity)
{
    TempCameraShakeIntensity = NewIntensity;
}

void UR1OptionsMenuWidget::SetTempConfineMouse(bool bEnabled)
{
    bTempConfineMouse = bEnabled;
}

void UR1OptionsMenuWidget::OnApplyButtonClicked()
{
    ApplyAndSaveSettings();
}

void UR1OptionsMenuWidget::OnCloseButtonClicked()
{
    if (AR1MainMenuController* MenuPC = Cast<AR1MainMenuController>(GetOwningPlayer()))
    {
        MenuPC->GoBack();
    }
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

    if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
    {
        TempResolution = UserSettings->GetScreenResolution();
        TempWindowMode = UserSettings->GetFullscreenMode();
        TempFrameRateLimit = UserSettings->GetFrameRateLimit();
        bTempVSyncEnabled = UserSettings->IsVSyncEnabled();
    }

    // Update child widgets
    if (WBP_Category_Graphics)
    {
        if (WBP_Category_Graphics->WBP_CheckBox_VSync) WBP_Category_Graphics->WBP_CheckBox_VSync->SetIsChecked(bTempVSyncEnabled);
    }

    if (WBP_Category_Audio)
    {
        if (WBP_Category_Audio->WBP_Slider_Master) WBP_Category_Audio->WBP_Slider_Master->SetValue(TempMasterVolume);
        if (WBP_Category_Audio->WBP_Slider_BGM) WBP_Category_Audio->WBP_Slider_BGM->SetValue(TempBGMVolume);
        if (WBP_Category_Audio->WBP_Slider_SFX) WBP_Category_Audio->WBP_Slider_SFX->SetValue(TempSFXVolume);
    }

    if (WBP_Category_Gameplay)
    {
        if (WBP_Category_Gameplay->WBP_Slider_MinimapOpacity) WBP_Category_Gameplay->WBP_Slider_MinimapOpacity->SetValue(TempMinimapOpacity);
        if (WBP_Category_Gameplay->WBP_CheckBox_ShowDamageText) WBP_Category_Gameplay->WBP_CheckBox_ShowDamageText->SetIsChecked(bTempShowDamageText);
    }

    if (WBP_Category_Controls)
    {
        if (WBP_Category_Controls->WBP_Slider_CameraShake) WBP_Category_Controls->WBP_Slider_CameraShake->SetValue(TempCameraShakeIntensity);
        if (WBP_Category_Controls->WBP_CheckBox_ConfineMouse) WBP_Category_Controls->WBP_CheckBox_ConfineMouse->SetIsChecked(bTempConfineMouse);
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

            // Graphics
            Settings->Resolution = TempResolution;
            Settings->WindowMode = TempWindowMode;
            Settings->FrameRateLimit = TempFrameRateLimit;
            Settings->bVSyncEnabled = bTempVSyncEnabled;

            SettingsSubsystem->SaveSettings();
        }
    }
}
