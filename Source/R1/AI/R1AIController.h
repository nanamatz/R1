

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "R1AIController.generated.h"

/**
 * 
 */
UCLASS()
class R1_API AR1AIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AR1AIController(const FObjectInitializer& ObjectInitializer);

	virtual void OnPossess(APawn * InPawn) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
