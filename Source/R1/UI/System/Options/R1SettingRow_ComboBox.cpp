#include "UI/System/Options/R1SettingRow_ComboBox.h"
#include "Components/TextBlock.h"
#include "Components/ComboBoxString.h"

void UR1SettingRow_ComboBox::NativeConstruct()
{
    Super::NativeConstruct();

    if (ComboBox_Value)
    {
        ComboBox_Value->OnSelectionChanged.AddDynamic(this, &UR1SettingRow_ComboBox::HandleInternalSelectionChanged);
    }
}

void UR1SettingRow_ComboBox::SetOptionName(const FText& InOptionName)
{
    if (Text_OptionName)
    {
        Text_OptionName->SetText(InOptionName);
    }
}

void UR1SettingRow_ComboBox::SetOptions(const TArray<FString>& InOptions)
{
    if (!ComboBox_Value) return;

    ComboBox_Value->ClearOptions();
    for (const FString& Option : InOptions)
    {
        ComboBox_Value->AddOption(Option);
    }
}

void UR1SettingRow_ComboBox::SetSelectedIndex(int32 Index)
{
    if (ComboBox_Value)
    {
        ComboBox_Value->SetSelectedIndex(Index);
    }
}

int32 UR1SettingRow_ComboBox::GetSelectedIndex() const
{
    if (!ComboBox_Value) return 0;
    return ComboBox_Value->GetSelectedIndex();
}

void UR1SettingRow_ComboBox::HandleInternalSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (!ComboBox_Value) return;
    int32 Index = ComboBox_Value->FindOptionIndex(SelectedItem);
    OnSelectionChanged.Broadcast(Index);
}
