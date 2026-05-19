#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "R1OptionsMenuWidget.generated.h"

UCLASS()
class R1_API UR1OptionsMenuWidget : public UR1UserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void SyncUIFromSettings();

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void ApplyAndSaveSettings();

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void OnApplyButtonClicked();

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void OnCloseButtonClicked();

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

protected:
    virtual void NativeConstruct() override;

    // Temporary variables for UI state (before Apply)
    UPROPERTY(BlueprintReadWrite, Category = "R1|UI|Temp")
    float TempMasterVolume;

    UPROPERTY(BlueprintReadWrite, Category = "R1|UI|Temp")
    float TempBGMVolume;

    UPROPERTY(BlueprintReadWrite, Category = "R1|UI|Temp")
    float TempSFXVolume;

    UPROPERTY(BlueprintReadWrite, Category = "R1|UI|Temp")
    bool bTempShowDamageText;

    UPROPERTY(BlueprintReadWrite, Category = "R1|UI|Temp")
    float TempMinimapOpacity;

    UPROPERTY(BlueprintReadWrite, Category = "R1|UI|Temp")
    bool bTempConfineMouse;

    UPROPERTY(BlueprintReadWrite, Category = "R1|UI|Temp")
    float TempCameraShakeIntensity;

    // --- Graphics Temp Variables ---
    UPROPERTY(BlueprintReadWrite, Category = "R1|UI|Temp|Graphics")
    FIntPoint TempResolution;

    UPROPERTY(BlueprintReadWrite, Category = "R1|UI|Temp|Graphics")
    TEnumAsByte<EWindowMode::Type> TempWindowMode;

    UPROPERTY(BlueprintReadWrite, Category = "R1|UI|Temp|Graphics")
    float TempFrameRateLimit;

    UPROPERTY(BlueprintReadWrite, Category = "R1|UI|Temp|Graphics")
    bool bTempVSyncEnabled;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UR1Category_Graphics> WBP_Category_Graphics;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UButton> Button_Apply;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UButton> Button_Close;
};
