#include "UI/System/Options/R1SettingRow_CheckBox.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"

void UR1SettingRow_CheckBox::NativeConstruct()
{
    Super::NativeConstruct();

    if (CheckBox_Value)
    {
        CheckBox_Value->OnCheckStateChanged.AddDynamic(this, &UR1SettingRow_CheckBox::HandleInternalCheckStateChanged);
    }
}

void UR1SettingRow_CheckBox::InitCheckBox(const FText& InOptionName, bool bInitialState)
{
    if (Text_OptionName)
    {
        Text_OptionName->SetText(InOptionName);
    }

    SetIsChecked(bInitialState);
}

void UR1SettingRow_CheckBox::SetIsChecked(bool bIsChecked)
{
    if (CheckBox_Value)
    {
        CheckBox_Value->SetCheckedState(bIsChecked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
    }
}

void UR1SettingRow_CheckBox::SetOptionName(const FText& InOptionName)
{
    if (Text_OptionName)
    {
        Text_OptionName->SetText(InOptionName);
    }
}

void UR1SettingRow_CheckBox::HandleInternalCheckStateChanged(bool bIsChecked)
{
    OnCheckStateChanged.Broadcast(bIsChecked);
}
