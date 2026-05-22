#include "System/R1SaveGame_Settings.h"

UR1SaveGame_Settings::UR1SaveGame_Settings()
{
    MasterVolume = 1.0f;
    BGMVolume = 1.0f;
    SFXVolume = 1.0f;
    bShowDamageText = true;
    MinimapOpacity = 0.5f;
    bConfineMouseToWindow = true;
    CameraShakeIntensity = 1.0f;

    Resolution = FIntPoint(1280, 720);
    WindowMode = EWindowMode::Windowed;
    FrameRateLimit = 60.0f;
    bVSyncEnabled = true;
}
