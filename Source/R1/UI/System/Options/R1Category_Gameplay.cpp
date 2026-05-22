
#include "UI/System/Options/R1Category_Gameplay.h"
#include "UI/System/Options/R1SettingRow_Slider.h"
#include "UI/System/Options/R1SettingRow_CheckBox.h"
#include "UI/System/Options/R1SettingRow_ComboBox.h"
#include "System/R1LocalizationSubsystem.h"

void UR1Category_Gameplay::NativeConstruct()
{
    Super::NativeConstruct();

    if (WBP_Slider_MinimapOpacity)
    {
        WBP_Slider_MinimapOpacity->DisplayMultiplier = 100.0f;
        WBP_Slider_MinimapOpacity->InitSlider(FText::GetEmpty(), 0.1f, 1.0f, 0.5f);
        WBP_Slider_MinimapOpacity->OnValueChanged.AddDynamic(this, &UR1Category_Gameplay::HandleMinimapOpacityChanged);
    }

    if (WBP_CheckBox_ShowDamageText)
    {
        WBP_CheckBox_ShowDamageText->InitCheckBox(FText::GetEmpty(), true);
        WBP_CheckBox_ShowDamageText->OnCheckStateChanged.AddDynamic(this, &UR1Category_Gameplay::HandleShowDamageTextChanged);
    }

    if (WBP_ComboBox_Language)
    {
        // Language names are invariant identity strings; they must show native script regardless of active language.
        WBP_ComboBox_Language->SetOptions({ TEXT("English"), TEXT("한국어") });
        WBP_ComboBox_Language->OnSelectionChanged.AddDynamic(this, &UR1Category_Gameplay::HandleLanguageSelectionChanged);
    }

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
        {
            LocSub->OnLanguageChanged.AddUObject(this, &UR1Category_Gameplay::RefreshLocalization);
        }
    }

    RefreshLocalization();
}

void UR1Category_Gameplay::NativeDestruct()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
        {
            LocSub->OnLanguageChanged.RemoveAll(this);
        }
    }
    Super::NativeDestruct();
}

void UR1Category_Gameplay::RefreshLocalization()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI) return;
    UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>();
    if (!LocSub) return;

    if (WBP_Slider_MinimapOpacity)
    {
        WBP_Slider_MinimapOpacity->SetOptionName(LocSub->GetText(TEXT("Option_MinimapOpacity")));
    }
    if (WBP_CheckBox_ShowDamageText)
    {
        WBP_CheckBox_ShowDamageText->SetOptionName(LocSub->GetText(TEXT("Option_FloatingDamage")));
    }
    if (WBP_ComboBox_Language)
    {
        WBP_ComboBox_Language->SetOptionName(LocSub->GetText(TEXT("Option_Language")));
    }
}

void UR1Category_Gameplay::SetSelectedLanguage(ER1Language Language)
{
    if (WBP_ComboBox_Language)
    {
        WBP_ComboBox_Language->SetSelectedIndex((int32)Language);
    }
}

void UR1Category_Gameplay::HandleMinimapOpacityChanged(float Value)
{
    OnMinimapOpacityChanged.Broadcast(Value);
}

void UR1Category_Gameplay::HandleShowDamageTextChanged(bool bIsChecked)
{
    OnShowDamageTextChanged.Broadcast(bIsChecked);
}

void UR1Category_Gameplay::HandleLanguageSelectionChanged(int32 SelectedIndex)
{
    OnLanguageChanged.Broadcast((ER1Language)SelectedIndex);
}
