

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_IsPlayerDead.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UBTService_IsPlayerDead : public UBTService
{
	GENERATED_BODY()
public:
	UBTService_IsPlayerDead();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBlackboardKeySelector TargetKey;
};
