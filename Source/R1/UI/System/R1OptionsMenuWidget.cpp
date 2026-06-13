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
#include "Components/TextBlock.h"

FR1SettingsSnapshot FR1SettingsSnapshot::FromSettings(const UR1SaveGame_Settings* Settings)
{
    FR1SettingsSnapshot Snapshot;
    if (Settings)
    {
        Snapshot.MasterVolume = Settings->MasterVolume;
        Snapshot.BGMVolume = Settings->BGMVolume;
        Snapshot.SFXVolume = Settings->SFXVolume;
        Snapshot.bShowDamageText = Settings->bShowDamageText;
        Snapshot.MinimapOpacity = Settings->MinimapOpacity;
        Snapshot.bConfineMouseToWindow = Settings->bConfineMouseToWindow;
        Snapshot.CameraShakeIntensity = Settings->CameraShakeIntensity;
        Snapshot.Language = Settings->Language;

        Snapshot.Resolution = Settings->Resolution;
        Snapshot.WindowMode = Settings->WindowMode;
        Snapshot.FrameRateLimit = Settings->FrameRateLimit;
        Snapshot.bVSyncEnabled = Settings->bVSyncEnabled;
    }
    return Snapshot;
}

void FR1SettingsSnapshot::ApplyTo(UR1SaveGame_Settings* Settings) const
{
    if (!Settings)
    {
        return;
    }

    Settings->MasterVolume = MasterVolume;
    Settings->BGMVolume = BGMVolume;
    Settings->SFXVolume = SFXVolume;
    Settings->bShowDamageText = bShowDamageText;
    Settings->MinimapOpacity = MinimapOpacity;
    Settings->bConfineMouseToWindow = bConfineMouseToWindow;
    Settings->CameraShakeIntensity = CameraShakeIntensity;
    Settings->Language = Language;

    Settings->Resolution = Resolution;
    Settings->WindowMode = WindowMode;
    Settings->FrameRateLimit = FrameRateLimit;
    Settings->bVSyncEnabled = bVSyncEnabled;
}

bool FR1SettingsSnapshot::NearlyEquals(const FR1SettingsSnapshot& Other) const
{
    if (!FMath::IsNearlyEqual(MasterVolume, Other.MasterVolume)) return false;
    if (!FMath::IsNearlyEqual(BGMVolume, Other.BGMVolume)) return false;
    if (!FMath::IsNearlyEqual(SFXVolume, Other.SFXVolume)) return false;
    if (bShowDamageText != Other.bShowDamageText) return false;
    if (!FMath::IsNearlyEqual(MinimapOpacity, Other.MinimapOpacity)) return false;
    if (bConfineMouseToWindow != Other.bConfineMouseToWindow) return false;
    if (!FMath::IsNearlyEqual(CameraShakeIntensity, Other.CameraShakeIntensity)) return false;
    if (Language != Other.Language) return false;

    if (Resolution != Other.Resolution) return false;
    if (WindowMode != Other.WindowMode) return false;
    if (!FMath::IsNearlyEqual(FrameRateLimit, Other.FrameRateLimit)) return false;
    if (bVSyncEnabled != Other.bVSyncEnabled) return false;

    return true;
}

void UR1OptionsMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 1. 먼저 해상도 목록을 생성하여 SupportedResolutions 배열을 채웁니다. (이 작업이 SyncUI보다 먼저 수행되어야 함)
    if (WBP_Category_Graphics)
    {
        WBP_Category_Graphics->InitResolutions(GenerateResolutionList());
        WBP_Category_Graphics->InitWindowModes({TEXT("Windowed"), TEXT("Fullscreen"), TEXT("Fullscreen Windowed")});

        WBP_Category_Graphics->OnResolutionSelected.AddDynamic(this, &UR1OptionsMenuWidget::SetTempResolutionByIndex);
        WBP_Category_Graphics->OnWindowModeSelected.AddDynamic(this, &UR1OptionsMenuWidget::SetTempWindowModeByIndex);
        WBP_Category_Graphics->OnVSyncChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempVSync);
        WBP_Category_Graphics->OnFPSChanged.AddDynamic(this, &UR1OptionsMenuWidget::SetTempFPS);
    }

    // 2. 이제 데이터로 Temp 변수를 채우고 UI 위젯들을 갱신합니다. (SupportedResolutions가 채워진 상태이므로 인덱스 찾기 가능)
    SyncUIFromSettings();

    // 3. 초기 상태를 저장하여 나중에 변경 사항이 있는지 확인하거나 롤백할 때 사용합니다.
    if (UR1SettingsSubsystem* SettingsSubsystem = GetGameInstance()->GetSubsystem<UR1SettingsSubsystem>())
    {
        if (UR1SaveGame_Settings* CurrentSettings = SettingsSubsystem->GetCustomSettings())
        {
            Original = FR1SettingsSnapshot::FromSettings(CurrentSettings);
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

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
        {
            LocSub->OnLanguageChanged.AddUObject(this, &UR1OptionsMenuWidget::RefreshLocalization);
        }
    }

    RefreshLocalization();
}

void UR1OptionsMenuWidget::NativeDestruct()
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

void UR1OptionsMenuWidget::RefreshLocalization()
{
    UGameInstance* GI = GetGameInstance();
    UR1LocalizationSubsystem* LocSub = GI ? GI->GetSubsystem<UR1LocalizationSubsystem>() : nullptr;
    if (!LocSub) return;

    if (Text_Graphics) Text_Graphics->SetText(LocSub->GetText("Tab_Graphics"));
    if (Text_Audio)    Text_Audio->SetText(LocSub->GetText("Tab_Audio"));
    if (Text_Gameplay) Text_Gameplay->SetText(LocSub->GetText("Tab_Gameplay"));
    if (Text_Controls) Text_Controls->SetText(LocSub->GetText("Tab_Controls"));
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
        Temp.Resolution = SupportedResolutions[SelectedIndex];
    }
}

void UR1OptionsMenuWidget::SetTempWindowModeByIndex(int32 SelectedIndex)
{
    switch (SelectedIndex)
    {
    case 0: Temp.WindowMode = EWindowMode::Windowed;          break;
    case 1: Temp.WindowMode = EWindowMode::Fullscreen;         break;
    case 2: Temp.WindowMode = EWindowMode::WindowedFullscreen; break;
    default: break;
    }
}

void UR1OptionsMenuWidget::SetTempVSync(bool bEnabled)
{
    Temp.bVSyncEnabled = bEnabled;
}

void UR1OptionsMenuWidget::SetTempFPS(float NewFPS)
{
    Temp.FrameRateLimit = NewFPS;
}

void UR1OptionsMenuWidget::SetTempMasterVolume(float NewVolume)
{
    Temp.MasterVolume = NewVolume;
}

void UR1OptionsMenuWidget::SetTempBGMVolume(float NewVolume)
{
    Temp.BGMVolume = NewVolume;
}

void UR1OptionsMenuWidget::SetTempSFXVolume(float NewVolume)
{
    Temp.SFXVolume = NewVolume;
}

void UR1OptionsMenuWidget::SetTempMinimapOpacity(float NewOpacity)
{
    Temp.MinimapOpacity = NewOpacity;
}

void UR1OptionsMenuWidget::SetTempShowDamageText(bool bEnabled)
{
    Temp.bShowDamageText = bEnabled;
}

void UR1OptionsMenuWidget::SetTempCameraShakeIntensity(float NewIntensity)
{
    Temp.CameraShakeIntensity = NewIntensity;
}

void UR1OptionsMenuWidget::SetTempConfineMouse(bool bEnabled)
{
    Temp.bConfineMouseToWindow = bEnabled;
}

void UR1OptionsMenuWidget::SetTempLanguage(ER1Language NewLanguage)
{
    Temp.Language = NewLanguage;
}

void UR1OptionsMenuWidget::OnDefaultsButtonClicked()
{
    // USaveGame 클래스의 기본값을 가져오기 위해 새 객체 생성
    UR1SaveGame_Settings* DefaultSettings = NewObject<UR1SaveGame_Settings>();
    if (DefaultSettings)
    {
        Temp = FR1SettingsSnapshot::FromSettings(DefaultSettings);

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
    // Original(최후의 저장 상태)과 현재 Temp 스냅샷을 비교
    return !Temp.NearlyEquals(Original);
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
            Temp = FR1SettingsSnapshot::FromSettings(Settings);
        }
    }

    // 그래픽 4종은 저장 파일이 아닌 엔진의 현재 상태를 기준으로 한다.
    if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
    {
        Temp.Resolution = UserSettings->GetScreenResolution();
        Temp.WindowMode = UserSettings->GetFullscreenMode();
        Temp.FrameRateLimit = UserSettings->GetFrameRateLimit();
        Temp.bVSyncEnabled = UserSettings->IsVSyncEnabled();
    }
}

void UR1OptionsMenuWidget::UpdateOriginalSettingsFromTemp()
{
    // 현재 Temp에 로드된 값을 '최초 원본'으로 복사
    Original = Temp;
}

void UR1OptionsMenuWidget::UpdateWidgetsFromTemp()
{
    // Update child widgets from Temp snapshot
    if (WBP_Category_Graphics)
    {
        if (WBP_Category_Graphics->WBP_CheckBox_VSync) WBP_Category_Graphics->WBP_CheckBox_VSync->SetIsChecked(Temp.bVSyncEnabled);
        if (WBP_Category_Graphics->WBP_Slider_FPS) WBP_Category_Graphics->WBP_Slider_FPS->SetValue(Temp.FrameRateLimit);

        // 창 모드 인덱스 계산 (콤보박스 순서: 0=Windowed, 1=Fullscreen, 2=WindowedFullscreen)
        int32 WindowModeIdx = 0;
        switch (Temp.WindowMode.GetValue())
        {
        case EWindowMode::Windowed:           WindowModeIdx = 0; break;
        case EWindowMode::Fullscreen:         WindowModeIdx = 1; break;
        case EWindowMode::WindowedFullscreen: WindowModeIdx = 2; break;
        }

        // 해상도 인덱스 찾기
        int32 ResIdx = SupportedResolutions.Find(Temp.Resolution);
        if (ResIdx == INDEX_NONE) ResIdx = 0;

        WBP_Category_Graphics->SetSelectedIndexes(ResIdx, WindowModeIdx);
    }

    if (WBP_Category_Audio)
    {
        if (WBP_Category_Audio->WBP_Slider_Master) WBP_Category_Audio->WBP_Slider_Master->SetValue(Temp.MasterVolume);
        if (WBP_Category_Audio->WBP_Slider_BGM) WBP_Category_Audio->WBP_Slider_BGM->SetValue(Temp.BGMVolume);
        if (WBP_Category_Audio->WBP_Slider_SFX) WBP_Category_Audio->WBP_Slider_SFX->SetValue(Temp.SFXVolume);
    }

    if (WBP_Category_Gameplay)
    {
        if (WBP_Category_Gameplay->WBP_Slider_MinimapOpacity) WBP_Category_Gameplay->WBP_Slider_MinimapOpacity->SetValue(Temp.MinimapOpacity);
        if (WBP_Category_Gameplay->WBP_CheckBox_ShowDamageText) WBP_Category_Gameplay->WBP_CheckBox_ShowDamageText->SetIsChecked(Temp.bShowDamageText);
        WBP_Category_Gameplay->SetSelectedLanguage(Temp.Language);
    }

    if (WBP_Category_Controls)
    {
        if (WBP_Category_Controls->WBP_Slider_CameraShake) WBP_Category_Controls->WBP_Slider_CameraShake->SetValue(Temp.CameraShakeIntensity);
        if (WBP_Category_Controls->WBP_CheckBox_ConfineMouse) WBP_Category_Controls->WBP_CheckBox_ConfineMouse->SetIsChecked(Temp.bConfineMouseToWindow);
    }
}

void UR1OptionsMenuWidget::ApplyAndSaveSettings(bool bSaveToDisk)
{
    if (UR1SettingsSubsystem* SettingsSubsystem = GetGameInstance()->GetSubsystem<UR1SettingsSubsystem>())
    {
        if (UR1SaveGame_Settings* Settings = SettingsSubsystem->GetCustomSettings())
        {
            Temp.ApplyTo(Settings);

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
                Original = Temp;
            }
        }
    }
}
