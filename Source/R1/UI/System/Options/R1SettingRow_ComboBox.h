#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1SettingRow_ComboBox.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboBoxRowSelectionChangedSignature, int32, SelectedIndex);

UCLASS()
class R1_API UR1SettingRow_ComboBox : public UR1UserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "R1|Events")
    FOnComboBoxRowSelectionChangedSignature OnSelectionChanged;

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void SetOptionName(const FText& InOptionName);

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void SetOptions(const TArray<FString>& InOptions);

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void SetSelectedIndex(int32 Index);

    UFUNCTION(BlueprintPure, Category = "R1|UI")
    int32 GetSelectedIndex() const;

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> Text_OptionName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UComboBoxString> ComboBox_Value;

private:
    UFUNCTION()
    void HandleInternalSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
};
