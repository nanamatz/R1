#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "System/R1LanguageTypes.h"
#include "R1OptionsMenuWidget.generated.h"

class UR1Category_Audio;
class UR1Category_Gameplay;
class UR1Category_Controls;
class UR1CommonButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOptionsCloseRequested);

// 옵션 화면에서 다루는 설정 값 한 벌(12종)의 인메모리 스냅샷.
// 직렬화 대상인 UR1SaveGame_Settings는 그대로 두고, 화면 내 임시 편집(Temp)과
// 변경 감지 기준(Original) 보관·복사·비교에만 사용한다.
struct FR1SettingsSnapshot
{
	// Audio
	float MasterVolume = 0.0f;
	float BGMVolume = 0.0f;
	float SFXVolume = 0.0f;

	// Gameplay / Controls
	bool bShowDamageText = false;
	float MinimapOpacity = 0.0f;
	bool bConfineMouseToWindow = false;
	float CameraShakeIntensity = 0.0f;
	ER1Language Language = ER1Language::English;

	// Graphics
	FIntPoint Resolution = FIntPoint::ZeroValue;
	TEnumAsByte<EWindowMode::Type> WindowMode = EWindowMode::Fullscreen;
	float FrameRateLimit = 0.0f;
	bool bVSyncEnabled = false;

	static FR1SettingsSnapshot FromSettings(const class UR1SaveGame_Settings* Settings);
	void ApplyTo(class UR1SaveGame_Settings* Settings) const;

	// 변경 감지용 비교. float는 IsNearlyEqual 기준 (기존 IsSettingsChanged와 동일 의미).
	bool NearlyEquals(const FR1SettingsSnapshot& Other) const;
};

UCLASS()
class R1_API UR1OptionsMenuWidget : public UR1UserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "R1|UI|Events")
    FOnOptionsCloseRequested OnCloseRequested;

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void SyncUIFromSettings();

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void LoadSettingsToTemp();

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void UpdateOriginalSettingsFromTemp();

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void UpdateWidgetsFromTemp();

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void ApplyAndSaveSettings(bool bSaveToDisk);

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    virtual void OnDefaultsButtonClicked();

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    virtual void OnApplyButtonClicked();

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    virtual void OnConfirmButtonClicked();

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    virtual void OnCancelButtonClicked();

    UFUNCTION()
    virtual void OnConfirmCancellation();

    UFUNCTION(BlueprintPure, Category = "R1|UI")
    bool IsSettingsChanged() const;

    UFUNCTION()
    virtual void OnCancelModalDismissed();
protected:
    UPROPERTY(EditDefaultsOnly, Category = "R1|UI")
    TSubclassOf<class UR1ConfirmModalSceneWidget> ConfirmModalClass;

    UPROPERTY()
    TObjectPtr<class UR1ConfirmModalSceneWidget> ActiveConfirmModalScene;
protected:
    // 모니터가 지원하는 실제 해상도(X, Y) 값을 보관할 배열
    TArray<FIntPoint> SupportedResolutions;

public:
    // UI 콤보박스를 채우기 위한 문자열 배열 반환 함수
    UFUNCTION(BlueprintCallable, Category = "R1|UI|Graphics")
    TArray<FString> GenerateResolutionList();

    // 콤보박스에서 선택한 인덱스를 기반으로 임시 해상도 설정
    UFUNCTION(BlueprintCallable, Category = "R1|UI|Graphics")
    void SetTempResolutionByIndex(int32 SelectedIndex);

protected:
    UFUNCTION()
    void SetTempWindowModeByIndex(int32 SelectedIndex);

    UFUNCTION()
    void SetTempVSync(bool bEnabled);

    UFUNCTION()
    void SetTempFPS(float NewFPS);

    UFUNCTION()
    void SetTempMasterVolume(float NewVolume);

    UFUNCTION()
    void SetTempBGMVolume(float NewVolume);

    UFUNCTION()
    void SetTempSFXVolume(float NewVolume);

    UFUNCTION()
    void SetTempMinimapOpacity(float NewOpacity);

    UFUNCTION()
    void SetTempShowDamageText(bool bEnabled);

    UFUNCTION()
    void SetTempCameraShakeIntensity(float NewIntensity);

    UFUNCTION()
    void SetTempConfineMouse(bool bEnabled);

    UFUNCTION()
    void SetTempLanguage(ER1Language NewLanguage);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    void RefreshLocalization();

protected:
    // 화면에서 편집 중인 값(Temp)과 변경 감지·롤백 기준점(Original).
    // (이전의 Temp* 개별 멤버 12종 + OriginalSettings 객체를 스냅샷 한 쌍으로 통합)
    FR1SettingsSnapshot Temp;
    FR1SettingsSnapshot Original;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UR1Category_Graphics> WBP_Category_Graphics;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UR1Category_Audio> WBP_Category_Audio;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UR1Category_Gameplay> WBP_Category_Gameplay;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UR1Category_Controls> WBP_Category_Controls;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Graphics;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Audio;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Gameplay;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Controls;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UR1CommonButton> Button_Defaults;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UR1CommonButton> Button_Apply;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UR1CommonButton> Button_Confirm;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UR1CommonButton> Button_Cancel;
};
