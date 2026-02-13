

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "R1PlayerSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1PlayerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	bool bPendingRespawn = false;

	UPROPERTY(BlueprintReadWrite)
	float MaxHealth = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float MaxMana = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float BaseDamage = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float BaseDefence = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float Exp = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float MaxExp = 0.f;
};
