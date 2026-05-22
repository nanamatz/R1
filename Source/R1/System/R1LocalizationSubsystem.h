#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "System/R1LanguageTypes.h"
#include "R1LocalizationSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnLanguageChanged);

UCLASS(Config=Game)
class R1_API UR1LocalizationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void SetLanguage(ER1Language NewLanguage);
    ER1Language GetLanguage() const { return CurrentLanguage; }

    FText GetText(FName Key) const;

    FOnLanguageChanged OnLanguageChanged;

private:
    UPROPERTY(Config)
    FSoftObjectPath LocalizationTablePath;

    UPROPERTY()
    mutable TObjectPtr<UDataTable> LocalizationTable;

    ER1Language CurrentLanguage = ER1Language::English;

    UDataTable* GetTable() const;
};
