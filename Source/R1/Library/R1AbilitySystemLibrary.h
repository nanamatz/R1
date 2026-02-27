

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "R1AbilitySystemLibrary.generated.h"

class AR1Monster;
/**
 * 
 */
UCLASS()
class R1_API UR1AbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION( Category = "Targeting")
	static TArray<AR1Character*> GetChainLightningTargets(AR1Character* SourceActor, AR1Character* InitialTarget, float BounceRadius, int32 MaxBounces);
};
