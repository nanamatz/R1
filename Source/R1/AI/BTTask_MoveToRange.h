

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_MoveToRange.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UBTTask_MoveToRange : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_MoveToRange();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	float RangeBuffer = 0.9f;
};
