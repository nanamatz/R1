#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "R1SettingsSubsystem.generated.h"

UCLASS()
class R1_API UR1SettingsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "R1|Settings")
    void ApplySettings();

    UFUNCTION(BlueprintCallable, Category = "R1|Settings")
    void SaveSettings();

    UFUNCTION(BlueprintCallable, Category = "R1|Settings")
    void LoadSettings();

    UFUNCTION(BlueprintPure, Category = "R1|Settings")
    class UR1SaveGame_Settings* GetCustomSettings() const { return CurrentSettings; }

private:
    UPROPERTY()
    TObjectPtr<class UR1SaveGame_Settings> CurrentSettings;

    const FString SettingsSaveSlotName = TEXT("Settings");
    const int32 SettingsUserIndex = 0;
public:
    void ApplyGraphicsSettings();
    void ApplyAudioSettings();
    void ApplyGameplaySettings();
    void ApplyControlSettings();
};
