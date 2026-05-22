
#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "System/R1LanguageTypes.h"
#include "R1Category_Gameplay.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinimapOpacityChangedSignature, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShowDamageTextChangedSignature, bool, bIsEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanguageChangedSignature, ER1Language, NewLanguage);

UCLASS()
class R1_API UR1Category_Gameplay : public UR1UserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable, Category = "R1|Events")
    FOnMinimapOpacityChangedSignature OnMinimapOpacityChanged;

    UPROPERTY(BlueprintAssignable, Category = "R1|Events")
    FOnShowDamageTextChangedSignature OnShowDamageTextChanged;

    UPROPERTY(BlueprintAssignable, Category = "R1|Events")
    FOnLanguageChangedSignature OnLanguageChanged;

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void SetSelectedLanguage(ER1Language Language);

public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UR1SettingRow_Slider> WBP_Slider_MinimapOpacity;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UR1SettingRow_CheckBox> WBP_CheckBox_ShowDamageText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UR1SettingRow_ComboBox> WBP_ComboBox_Language;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    void RefreshLocalization();

    UFUNCTION()
    void HandleMinimapOpacityChanged(float Value);

    UFUNCTION()
    void HandleShowDamageTextChanged(bool bIsChecked);

    UFUNCTION()
    void HandleLanguageSelectionChanged(int32 SelectedIndex);
};
