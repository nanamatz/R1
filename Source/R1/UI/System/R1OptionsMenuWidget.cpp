#include "UI/System/R1OptionsMenuWidget.h"
#include "System/R1SettingsSubsystem.h"
#include "System/R1LocalizationSubsystem.h"
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
#include "UI/R1CommonButton.h"
#include "UI/System/R1ConfirmModal.h"
#include "UI/System/R1ConfirmModalSceneWidget.h"
#include "Components/Button.h"

void UR1OptionsMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 1. 먼저 해상도 목록을 생성하여 SupportedResolutions 배열을 채웁니다. (이 작업이 SyncUI보다 먼저 수행되어야 함)
    if (WBP_Category_Graphics)
    {
        WBP_Category_Graphics->InitResolutions(GenerateResolutionList());

        WBP_Category_Graphics->OnResolutionSelected.AddDynamic(this, &UR1OptionsMenuWidget::SetTempResolutionByIndex);
        WBP_Category_Graphics->OnWindowModeSelected.AddDynamic(this, &UR1OptionsMenuWidget::SetTempWindowModeByIndex);
        WBP_Category_Graphics->OnVSyncChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempVSync);
        WBP_Category_Graphics->OnFPSChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempFPS);
    }

    // 2. 이제 데이터로 Temp 변수를 채우고 UI 위젯들을 갱신합니다. (SupportedResolutions가 채워진 상태이므로 인덱스 찾기 가능)
    SyncUIFromSettings();

    // 3. 초기 상태를 저장하여 나중에 변경 사항이 있는지 확인하거나 롤백할 때 사용합니다.
    if (!OriginalSettings)
    {
        OriginalSettings = NewObject<UR1SaveGame_Settings>(this);
    }
    
    if (UR1SettingsSubsystem* SettingsSubsystem = GetGameInstance()->GetSubsystem<UR1SettingsSubsystem>())
    {
        if (UR1SaveGame_Settings* CurrentSettings = SettingsSubsystem->GetCustomSettings())
        {
            OriginalSettings->MasterVolume = CurrentSettings->MasterVolume;
            OriginalSettings->BGMVolume = CurrentSettings->BGMVolume;
            OriginalSettings->SFXVolume = CurrentSettings->SFXVolume;
            OriginalSettings->bShowDamageText = CurrentSettings->bShowDamageText;
            OriginalSettings->MinimapOpacity = CurrentSettings->MinimapOpacity;
            OriginalSettings->bConfineMouseToWindow = CurrentSettings->bConfineMouseToWindow;
            OriginalSettings->CameraShakeIntensity = CurrentSettings->CameraShakeIntensity;
            
            OriginalSettings->Resolution = CurrentSettings->Resolution;
            OriginalSettings->WindowMode = CurrentSettings->WindowMode;
            OriginalSettings->FrameRateLimit = CurrentSettings->FrameRateLimit;
            OriginalSettings->bVSyncEnabled = CurrentSettings->bVSyncEnabled;
            OriginalSettings->Language = CurrentSettings->Language;
        }
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
        WBP_Category_Gameplay->OnLanguageChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempLanguage);
    }

    if (WBP_Category_Controls)
    {
        WBP_Category_Controls->OnCameraShakeChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempCameraShakeIntensity);
        WBP_Category_Controls->OnConfineMouseChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempConfineMouse);
    }

    if (Button_Defaults && Button_Defaults->CommonButton)
    {
        Button_Defaults->CommonButton->OnClicked.AddDynamic(this, &UR1OptionsMenuWidget::OnDefaultsButtonClicked);
    }

    if (Button_Apply && Button_Apply->CommonButton)
    {
        Button_Apply->CommonButton->OnClicked.AddDynamic(this, &UR1OptionsMenuWidget::OnApplyButtonClicked);
    }

    if (Button_Confirm && Button_Confirm->CommonButton)
    {
        Button_Confirm->CommonButton->OnClicked.AddDynamic(this, &UR1OptionsMenuWidget::OnConfirmButtonClicked);
    }

    if (Button_Cancel)
    {
        Button_Cancel->CommonButton->OnClicked.AddDynamic(this, &UR1OptionsMenuWidget::OnCancelButtonClicked);
    }
}


TArray<FString> UR1OptionsMenuWidget::GenerateResolutionList()
{
    TArray<FString> StringList;
    TArray<FIntPoint> RawResolutions;
    SupportedResolutions.Empty();

    // 1. 현재 모니터가 지원하는 전체화면 해상도 목록 가져오기
    UKismetSystemLibrary::GetSupportedFullscreenResolutions(RawResolutions);

    TArray<FIntPoint> StandardResolutions = {
        FIntPoint(1280, 720),   // HD
        FIntPoint(1920, 1080),  // FHD
        FIntPoint(2560, 1440),  // QHD
    };

    // 2. 만약 엔진에서 목록을 가져오지 못했다면 안전을 위한 기본값 세팅
    if (RawResolutions.Num() == 0)
    {
        RawResolutions.Add(FIntPoint(1920, 1080));
        RawResolutions.Add(FIntPoint(2560, 1440));
    }

    // 3. 필터링: 모니터가 지원하는 해상도 중, 표준 목록에 있는 것만 추출
    for (const FIntPoint& StandardRes : StandardResolutions)
    {
        if (RawResolutions.Contains(StandardRes))
        {
            SupportedResolutions.Add(StandardRes);

            // UI 콤보박스에 표시할 예쁜 문자열 생성
            FString ResString = FString::Printf(TEXT("%d x %d"), StandardRes.X, StandardRes.Y);
            StringList.Add(ResString);
        }
    }

    // 4. 안전 장치 (만약 모니터가 너무 특이해서 필터링 후 목록이 텅 비어버렸을 경우)
    if (StringList.Num() == 0)
    {
        SupportedResolutions.Empty();
        SupportedResolutions.Add(FIntPoint(1920, 1080));
        StringList.Add(TEXT("1920 x 1080"));
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

void UR1OptionsMenuWidget::SetTempLanguage(ER1Language NewLanguage)
{
    TempLanguage = NewLanguage;
}

void UR1OptionsMenuWidget::OnDefaultsButtonClicked()
{
    // USaveGame 클래스의 기본값을 가져오기 위해 새 객체 생성
    UR1SaveGame_Settings* DefaultSettings = NewObject<UR1SaveGame_Settings>();
    if (DefaultSettings)
    {
        TempMasterVolume = DefaultSettings->MasterVolume;
        TempBGMVolume = DefaultSettings->BGMVolume;
        TempSFXVolume = DefaultSettings->SFXVolume;
        bTempShowDamageText = DefaultSettings->bShowDamageText;
        TempMinimapOpacity = DefaultSettings->MinimapOpacity;
        bTempConfineMouse = DefaultSettings->bConfineMouseToWindow;
        TempCameraShakeIntensity = DefaultSettings->CameraShakeIntensity;

        TempResolution = DefaultSettings->Resolution;
        TempWindowMode = DefaultSettings->WindowMode;
        TempFrameRateLimit = DefaultSettings->FrameRateLimit;
        bTempVSyncEnabled = DefaultSettings->bVSyncEnabled;
        TempLanguage = DefaultSettings->Language;

        // UI 갱신
        UpdateWidgetsFromTemp();
    }
}

void UR1OptionsMenuWidget::OnApplyButtonClicked()
{
    // Apply 버튼: 파일 저장은 하지 않고 엔진 상태만 업데이트 (임시 적용)
    ApplyAndSaveSettings(false);
}

void UR1OptionsMenuWidget::OnConfirmButtonClicked()
{
    // Confirm 버튼: 엔진 상태 업데이트 및 파일 저장 수행 후 닫기
    ApplyAndSaveSettings(true);
    OnCloseRequested.Broadcast();
}

void UR1OptionsMenuWidget::OnCancelButtonClicked()
{
    if (IsSettingsChanged())
    {
        if (ConfirmModalClass)
        {
            // 이미 모달이 떠 있다면 중복 생성 방지
            if (ActiveConfirmModalScene) return;

            ActiveConfirmModalScene = CreateWidget<UR1ConfirmModalSceneWidget>(this, ConfirmModalClass);
            if (ActiveConfirmModalScene)
            {
                if (ActiveConfirmModalScene->WBP_ConfirmModal)
                {
                    if (UGameInstance* GI = GetGameInstance())
                    {
                        if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
                        {
                            ActiveConfirmModalScene->WBP_ConfirmModal->SetMessage(LocSub->GetText(TEXT("Modal_UnsavedChanges")));
                        }
                    }

                    ActiveConfirmModalScene->WBP_ConfirmModal->OnConfirm.AddDynamic(this, &UR1OptionsMenuWidget::OnConfirmCancellation);
                    ActiveConfirmModalScene->WBP_ConfirmModal->OnCancel.AddDynamic(this, &UR1OptionsMenuWidget::OnCancelModalDismissed);
                }

                // 모달 알맹이가 아닌, 래핑된 SceneWidget 전체를 화면에 띄움
                ActiveConfirmModalScene->AddToViewport(100);
                return;
            }
        }
    }

    // 변경사항이 없거나 모달 생성 실패 시 즉시 옵션창 종료
    OnConfirmCancellation();
}

void UR1OptionsMenuWidget::OnConfirmCancellation()
{
    // 1. 화면에 남아있는 껍데기(SceneWidget) 전체를 뷰포트에서 완전히 파괴
    if (ActiveConfirmModalScene)
    {
        ActiveConfirmModalScene->RemoveFromParent();
        ActiveConfirmModalScene = nullptr;
    }

    // 2. 변경사항 롤백
    if (UR1SettingsSubsystem* SettingsSubsystem = GetGameInstance()->GetSubsystem<UR1SettingsSubsystem>())
    {
        // 파일(Saved)에서 다시 읽어와서 엔진에 즉시 재적용
        SettingsSubsystem->LoadSettings();

        // 3. UI에도 롤백된 값을 반영 (사용자 요청 사항)
        SyncUIFromSettings();
    }

    // 4. 옵션창 닫기 요청
    OnCloseRequested.Broadcast();
}

bool UR1OptionsMenuWidget::IsSettingsChanged() const
{
    if (!OriginalSettings) return false;

    // OriginalSettings(최후의 저장 상태)와 현재 Temp 변수들을 비교
    if (!FMath::IsNearlyEqual(TempMasterVolume, OriginalSettings->MasterVolume)) return true;
    if (!FMath::IsNearlyEqual(TempBGMVolume, OriginalSettings->BGMVolume)) return true;
    if (!FMath::IsNearlyEqual(TempSFXVolume, OriginalSettings->SFXVolume)) return true;
    if (bTempShowDamageText != OriginalSettings->bShowDamageText) return true;
    if (!FMath::IsNearlyEqual(TempMinimapOpacity, OriginalSettings->MinimapOpacity)) return true;
    if (bTempConfineMouse != OriginalSettings->bConfineMouseToWindow) return true;
    if (!FMath::IsNearlyEqual(TempCameraShakeIntensity, OriginalSettings->CameraShakeIntensity)) return true;

    if (TempResolution != OriginalSettings->Resolution) return true;
    if (TempWindowMode != OriginalSettings->WindowMode) return true;
    if (!FMath::IsNearlyEqual(TempFrameRateLimit, OriginalSettings->FrameRateLimit)) return true;
    if (bTempVSyncEnabled != OriginalSettings->bVSyncEnabled) return true;
    if (TempLanguage != OriginalSettings->Language) return true;

    return false;
}

void UR1OptionsMenuWidget::OnCancelModalDismissed()
{
    // 모달에서 '취소(돌아가기)'를 누른 경우: SceneWidget만 파괴하고 옵션창은 그대로 유지하여 세팅을 계속하게 함
    if (ActiveConfirmModalScene)
    {
        ActiveConfirmModalScene->RemoveFromParent();
        ActiveConfirmModalScene = nullptr;
    }
}

void UR1OptionsMenuWidget::SyncUIFromSettings()
{
    LoadSettingsToTemp();
    UpdateOriginalSettingsFromTemp(); // 매번 로드 시점의 값을 원본으로 기록 (변경 감지 기준점)
    UpdateWidgetsFromTemp();
}

void UR1OptionsMenuWidget::LoadSettingsToTemp()
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
            TempLanguage = Settings->Language;
        }
    }

    if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
    {
        TempResolution = UserSettings->GetScreenResolution();
        TempWindowMode = UserSettings->GetFullscreenMode();
        TempFrameRateLimit = UserSettings->GetFrameRateLimit();
        bTempVSyncEnabled = UserSettings->IsVSyncEnabled();
    }
}

void UR1OptionsMenuWidget::UpdateOriginalSettingsFromTemp()
{
    if (!OriginalSettings)
    {
        OriginalSettings = NewObject<UR1SaveGame_Settings>(this);
    }

    // 현재 Temp에 로드된 값을 '최초 원본'으로 복사
    OriginalSettings->MasterVolume = TempMasterVolume;
    OriginalSettings->BGMVolume = TempBGMVolume;
    OriginalSettings->SFXVolume = TempSFXVolume;
    OriginalSettings->bShowDamageText = bTempShowDamageText;
    OriginalSettings->MinimapOpacity = TempMinimapOpacity;
    OriginalSettings->bConfineMouseToWindow = bTempConfineMouse;
    OriginalSettings->CameraShakeIntensity = TempCameraShakeIntensity;

    OriginalSettings->Resolution = TempResolution;
    OriginalSettings->WindowMode = TempWindowMode;
    OriginalSettings->FrameRateLimit = TempFrameRateLimit;
    OriginalSettings->bVSyncEnabled = bTempVSyncEnabled;
    OriginalSettings->Language = TempLanguage;
}

void UR1OptionsMenuWidget::UpdateWidgetsFromTemp()
{
    // Update child widgets from Temp variables
    if (WBP_Category_Graphics)
    {
        if (WBP_Category_Graphics->WBP_CheckBox_VSync) WBP_Category_Graphics->WBP_CheckBox_VSync->SetIsChecked(bTempVSyncEnabled);
        if (WBP_Category_Graphics->WBP_Slider_FPS) WBP_Category_Graphics->WBP_Slider_FPS->SetValue(TempFrameRateLimit);

        // 창 모드 인덱스 계산 (EWindowMode::Fullscreen=0, WindowedFullscreen=1, Windowed=2)
        int32 WindowModeIdx = (int32)TempWindowMode.GetValue();

        // 해상도 인덱스 찾기
        int32 ResIdx = SupportedResolutions.Find(TempResolution);
        if (ResIdx == INDEX_NONE) ResIdx = 0;

        WBP_Category_Graphics->SetSelectedIndexes(ResIdx, WindowModeIdx);
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
        WBP_Category_Gameplay->SetSelectedLanguage(TempLanguage);
    }

    if (WBP_Category_Controls)
    {
        if (WBP_Category_Controls->WBP_Slider_CameraShake) WBP_Category_Controls->WBP_Slider_CameraShake->SetValue(TempCameraShakeIntensity);
        if (WBP_Category_Controls->WBP_CheckBox_ConfineMouse) WBP_Category_Controls->WBP_CheckBox_ConfineMouse->SetIsChecked(bTempConfineMouse);
    }
}

void UR1OptionsMenuWidget::ApplyAndSaveSettings(bool bSaveToDisk)
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
            Settings->Language = TempLanguage;

            // 서브시스템에 현재 Temp 값들을 적용하라고 명령 (Subsystem 내부에 Apply 로직이 있어야 함)
            SettingsSubsystem->ApplyGraphicsSettings();
            SettingsSubsystem->ApplyAudioSettings();
            SettingsSubsystem->ApplyGameplaySettings();
            SettingsSubsystem->ApplyControlSettings();

            // Confirm 버튼인 경우에만 디스크에 저장
            if (bSaveToDisk)
            {
                SettingsSubsystem->SaveSettings();

                // 🌟 저장 성공 후에는 현재 상태가 다시 '최신 원본'이 됩니다.
                if (OriginalSettings)
                {
                    OriginalSettings->MasterVolume = Settings->MasterVolume;
                    OriginalSettings->BGMVolume = Settings->BGMVolume;
                    OriginalSettings->SFXVolume = Settings->SFXVolume;
                    OriginalSettings->bShowDamageText = Settings->bShowDamageText;
                    OriginalSettings->MinimapOpacity = Settings->MinimapOpacity;
                    OriginalSettings->bConfineMouseToWindow = Settings->bConfineMouseToWindow;
                    OriginalSettings->CameraShakeIntensity = Settings->CameraShakeIntensity;

                    OriginalSettings->Resolution = Settings->Resolution;
                    OriginalSettings->WindowMode = Settings->WindowMode;
                    OriginalSettings->FrameRateLimit = Settings->FrameRateLimit;
                    OriginalSettings->bVSyncEnabled = Settings->bVSyncEnabled;
                    OriginalSettings->Language = Settings->Language;
                }
            }
        }
    }
}
