#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "R1Define.h"
#include "R1DamageTextWidget.generated.h"

UCLASS()
class R1_API UR1DamageTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Damage UI")
	void SetDamageInfo(const FR1DamageInfo& Info);

	UFUNCTION(BlueprintImplementableEvent, Category = "Damage UI")
	void OnSetDamageInfo(const FR1DamageInfo& Info);

	void ReturnToPool();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage UI")
	TObjectPtr<class UWidgetAnimation> FloatAnim;

	UFUNCTION(BlueprintCallable, Category = "Damage UI")
	void HandleAnimationFinished();
};
