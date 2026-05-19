#include "UI/System/R1OptionsMenuWidget.h"
#include "System/R1SettingsSubsystem.h"
#include "System/R1SaveGame_Settings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/GameUserSettings.h"
#include "UI/System/Options/R1Category_Graphics.h"
#include "UI/R1HUD.h"
#include "Components/Button.h"

void UR1OptionsMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SyncUIFromSettings();

    if (WBP_Category_Graphics)
    {
        WBP_Category_Graphics->InitResolutions(GenerateResolutionList());

        WBP_Category_Graphics->OnResolutionSelected.AddDynamic(this, &UR1OptionsMenuWidget::ApplyResolutionByIndex);
    }

    if (Btn_Apply)
    {
        Btn_Apply->OnClicked.AddDynamic(this, &UR1OptionsMenuWidget::OnApplyButtonClicked);
    }

    if (Btn_Close)
    {
        Btn_Close->OnClicked.AddDynamic(this, &UR1OptionsMenuWidget::OnCloseButtonClicked);
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

void UR1OptionsMenuWidget::ApplyResolutionByIndex(int32 SelectedIndex)
{
    // 유효한 인덱스인지 안전 검사
    if (SupportedResolutions.IsValidIndex(SelectedIndex))
    {
        if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
        {
            // 1. 엔진 세팅 객체에 해상도 값 입력
            UserSettings->SetScreenResolution(SupportedResolutions[SelectedIndex]);

            // 2. 화면에 즉시 적용 (인자 값 false: ini 파일에 영구 저장하지는 않음)
            UserSettings->ApplyResolutionSettings(false);
        }
    }
}

void UR1OptionsMenuWidget::OnApplyButtonClicked()
{
    ApplyAndSaveSettings();
}

void UR1OptionsMenuWidget::OnCloseButtonClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (AR1HUD* HUD = Cast<AR1HUD>(PC->GetHUD()))
        {
            HUD->ToggleOptionsUI();
        }
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
