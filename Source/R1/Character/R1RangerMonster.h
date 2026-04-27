
#pragma once

#include "CoreMinimal.h"
#include "Character/R1Monster.h"
#include "R1RangerMonster.generated.h"

/**
 * 
 */
UCLASS()
class R1_API AR1RangerMonster : public AR1Monster
{
	GENERATED_BODY()

public:
    AR1RangerMonster();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TSubclassOf<class AR1Projectile> ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FName MuzzleSocketName = FName("Muzzle_Front");
	
};
