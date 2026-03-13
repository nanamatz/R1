

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_MoveAway.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UBTTask_MoveAway : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_MoveAway();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// 얼마나 멀리 도망갈 것인지 (에디터에서 수정 가능)
	UPROPERTY(EditAnywhere, Category = "AI")
	float FleeDistance = 500.0f;
};
