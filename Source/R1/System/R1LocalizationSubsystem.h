#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "System/R1LanguageTypes.h"
#include "R1LocalizationSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnLanguageChanged);

UCLASS()
class R1_API UR1LocalizationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    void SetLanguage(ER1Language NewLanguage);
    ER1Language GetLanguage() const { return CurrentLanguage; }

    FText GetText(FName Key);

    FOnLanguageChanged OnLanguageChanged;

private:
    UPROPERTY()
    TObjectPtr<UDataTable> LocalizationTable;

    ER1Language CurrentLanguage = ER1Language::English;
};
