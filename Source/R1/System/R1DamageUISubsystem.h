#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "R1Define.h"
#include "R1DamageUISubsystem.generated.h"

class UR1DamageTextWidget;

UCLASS()
class R1_API UR1DamageUISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Damage UI")
	void ShowDamageText(const FR1DamageInfo& DamageInfo);

	void ReturnWidgetToPool(UR1DamageTextWidget* Widget);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Damage UI")
	TSubclassOf<UR1DamageTextWidget> DamageWidgetClass;

private:
	UPROPERTY()
	TArray<TObjectPtr<UR1DamageTextWidget>> WidgetPool;

	UR1DamageTextWidget* GetWidgetFromPool();
};
