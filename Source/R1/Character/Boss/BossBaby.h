

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

	// MuzzleSocketName은 AR1Monster로 올라갔다. 베이스 기본값은 "Muzzle_Front"이므로
	// BP_Baby에서 "Muzzle_02"로 덮어쓸 것.
};
