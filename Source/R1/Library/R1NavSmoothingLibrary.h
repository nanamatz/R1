#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "R1NavSmoothingLibrary.generated.h"

/**
 * Player movement helper: queries the navmesh, string-pulls the path with NavRaycast
 * (drops midpoints that have a clear on-navmesh straight line), and issues the move.
 * Falls back to stock SimpleMoveToLocation when there is nothing to smooth or any input is invalid.
 */
UCLASS()
class R1_API UR1NavSmoothingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	static void SmoothMoveTo(AController* Controller, const FVector& Destination);
};
