

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "R1ExpBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1ExpBarWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UR1ExpBarWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION()
	void UpdateExpBar(float Ratio);
protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	TObjectPtr<class UProgressBar> ExpBar;

private:
	float TargetPercent;
};
