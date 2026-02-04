

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "R1GameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UR1GameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

};
