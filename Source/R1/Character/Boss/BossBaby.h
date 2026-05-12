

#pragma once

#include "CoreMinimal.h"
#include "Character/R1Boss.h"
#include "BossBaby.generated.h"

/**
 * 
 */
UCLASS()
class R1_API ABossBaby : public AR1Boss
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName MuzzleSocketName = FName("Muzzle_02");
};
