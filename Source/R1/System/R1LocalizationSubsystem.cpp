#include "System/R1LocalizationSubsystem.h"
#include "System/R1LocTextRow.h"
#include "Engine/DataTable.h"

void UR1LocalizationSubsystem::SetLanguage(ER1Language NewLanguage)
{
    if (CurrentLanguage == NewLanguage) return;
    CurrentLanguage = NewLanguage;
    OnLanguageChanged.Broadcast();
}

FText UR1LocalizationSubsystem::GetText(FName Key) const
{
    UDataTable* Table = GetTable();
    if (!Table)
    {
        return FText::FromName(Key);
    }

    const FR1LocTextRow* Row = Table->FindRow<FR1LocTextRow>(Key, TEXT("UR1LocalizationSubsystem::GetText"));
    if (!Row)
    {
        return FText::FromName(Key);
    }

    return (CurrentLanguage == ER1Language::Korean) ? Row->Korean : Row->English;
}

UDataTable* UR1LocalizationSubsystem::GetTable() const
{
    if (LocalizationTable)
    {
        return LocalizationTable;
    }

    if (!LocalizationTablePath.IsNull())
    {
        LocalizationTable = Cast<UDataTable>(LocalizationTablePath.TryLoad());
        if (!LocalizationTable)
        {
            UE_LOG(LogTemp, Error, TEXT("UR1LocalizationSubsystem: Failed to load localization table at '%s'"), *LocalizationTablePath.ToString());
            LocalizationTablePath.Reset();
        }
        return LocalizationTable;
    }

    return nullptr;
}
