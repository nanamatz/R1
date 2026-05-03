#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "R1Define.h"
#include "R1DamageTextWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDamageAnimFinishedSignature);

UCLASS()
class R1_API UR1DamageTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Damage UI")
	void SetDamageInfo(const FR1DamageInfo& Info);

	UFUNCTION(BlueprintImplementableEvent, Category = "Damage UI")
	void OnSetDamageInfo(const FR1DamageInfo& Info);

	UPROPERTY(BlueprintAssignable, Category = "Damage UI")
	FOnDamageAnimFinishedSignature OnAnimationFinished;

protected:
	UPROPERTY(BlueprintReadWrite,meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_DamageAmount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage UI")
	TObjectPtr<class UWidgetAnimation> FloatAnim;

	UFUNCTION(BlueprintCallable, Category = "Damage UI")
	void HandleAnimationFinished();
};
