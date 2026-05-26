

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1CommonButton.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1CommonButton : public UR1UserWidget
{
	GENERATED_BODY()

public:
	virtual void SynchronizeProperties() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ExposeOnSpawn = true))
	FText ButtonText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ExposeOnSpawn = true))
	FName LocalizationKey;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> CommonButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ButtonName;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void RefreshLocalization();
};
