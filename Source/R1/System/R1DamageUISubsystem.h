#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "R1Define.h"
#include "R1DamageUISubsystem.generated.h"

class AR1DamageTextActor;

UCLASS()
class R1_API UR1DamageUISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UR1DamageUISubsystem();

public:
	UFUNCTION(BlueprintCallable, Category = "Damage UI")
	void ShowDamageText(const FR1DamageInfo& DamageInfo);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Damage UI")
	TSubclassOf<AR1DamageTextActor> DamageActorClass;
};
