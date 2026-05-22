#include "System/R1LocalizationSubsystem.h"
#include "System/R1LocTextRow.h"
#include "Engine/DataTable.h"

void UR1LocalizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LocalizationTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/DataTable/DT_Localization.DT_Localization"));
    if (!LocalizationTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[LocSub] Failed to load DT_Localization — create the asset in Content/DataTable/"));
    }
}

void UR1LocalizationSubsystem::SetLanguage(ER1Language NewLanguage)
{
    UE_LOG(LogTemp, Warning, TEXT("[LocSub] SetLanguage called: %d -> %d"), (int32)CurrentLanguage, (int32)NewLanguage);
    if (CurrentLanguage == NewLanguage) return;
    CurrentLanguage = NewLanguage;
    OnLanguageChanged.Broadcast();
}

FText UR1LocalizationSubsystem::GetText(FName Key)
{
    if (!LocalizationTable)
    {
        return FText::FromName(Key);
    }
    const FR1LocTextRow* Row = LocalizationTable->FindRow<FR1LocTextRow>(Key, TEXT("UR1LocalizationSubsystem::GetText"));
    if (!Row)
    {
        return FText::FromName(Key);
    }
    return (CurrentLanguage == ER1Language::Korean) ? Row->Korean : Row->English;
}
