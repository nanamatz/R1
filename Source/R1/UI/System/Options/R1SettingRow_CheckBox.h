#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1SettingRow_CheckBox.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCheckBoxRowStateChangedSignature, bool, bIsChecked);

UCLASS()
class R1_API UR1SettingRow_CheckBox : public UR1UserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "R1|Events")
    FOnCheckBoxRowStateChangedSignature OnCheckStateChanged;

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void InitCheckBox(const FText& InOptionName, bool bInitialState);

    UFUNCTION(BlueprintCallable, Category = "R1|UI")
    void SetIsChecked(bool bIsChecked);

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> Text_OptionName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UCheckBox> CheckBox_Value;

private:
    UFUNCTION()
    void HandleInternalCheckStateChanged(bool bIsChecked);
};
