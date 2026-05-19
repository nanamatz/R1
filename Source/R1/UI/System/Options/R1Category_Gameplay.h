
#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1Category_Gameplay.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinimapOpacityChangedSignature, float, NewValue);

UCLASS()
class R1_API UR1Category_Gameplay : public UR1UserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnMinimapOpacityChangedSignature OnMinimapOpacityChanged;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1SettingRow_Slider> WBP_Slider_MinimapOpacity;

private:
	UFUNCTION()
	void HandleMinimapOpacityChanged(float Value);
};
