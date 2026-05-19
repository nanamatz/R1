
#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1Category_Audio.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMasterVolumeChangedSignature, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBGMVolumeChangedSignature, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSFXVolumeChangedSignature, float, NewValue);

UCLASS()
class R1_API UR1Category_Audio : public UR1UserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnMasterVolumeChangedSignature OnMasterVolumeChanged;

	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnBGMVolumeChangedSignature OnBGMVolumeChanged;

	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnSFXVolumeChangedSignature OnSFXVolumeChanged;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1SettingRow_Slider> WBP_Slider_Master;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1SettingRow_Slider> WBP_Slider_BGM;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1SettingRow_Slider> WBP_Slider_SFX;

private:
	UFUNCTION()
	void HandleMasterVolumeChanged(float Value);

	UFUNCTION()
	void HandleBGMVolumeChanged(float Value);

	UFUNCTION()
	void HandleSFXVolumeChanged(float Value);
};
