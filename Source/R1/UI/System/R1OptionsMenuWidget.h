#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
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
};
