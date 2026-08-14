
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

    // ProjectileClass / MuzzleSocketName은 AR1Monster로 올라갔다 (보스도 사용해야 하므로).
};
