#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "R1SaveGame_Settings.generated.h"

UCLASS()
class R1_API UR1SaveGame_Settings : public USaveGame
{
    GENERATED_BODY()
public:
    UR1SaveGame_Settings();

    // --- Audio ---
    UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
    float MasterVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
    float BGMVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
    float SFXVolume = 1.0f;

    // --- Gameplay ---
    UPROPERTY(BlueprintReadWrite, Category = "Settings|Gameplay")
    bool bShowDamageText = true;

    UPROPERTY(BlueprintReadWrite, Category = "Settings|Gameplay")
    float MinimapOpacity = 0.5f;

    // --- Controls/Accessibility ---
    UPROPERTY(BlueprintReadWrite, Category = "Settings|Controls")
    bool bConfineMouseToWindow = true;

    UPROPERTY(BlueprintReadWrite, Category = "Settings|Controls")
    float CameraShakeIntensity = 1.0f;

    // --- Graphics ---
    UPROPERTY(BlueprintReadWrite, Category = "Settings|Graphics")
    FIntPoint Resolution = FIntPoint(1920, 1080);

    UPROPERTY(BlueprintReadWrite, Category = "Settings|Graphics")
    TEnumAsByte<EWindowMode::Type> WindowMode = EWindowMode::WindowedFullscreen;

    UPROPERTY(BlueprintReadWrite, Category = "Settings|Graphics")
    float FrameRateLimit = 60.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Settings|Graphics")
    bool bVSyncEnabled = true;
};
