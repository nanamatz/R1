

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SpecialAttack.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UBTTask_SpecialAttack : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_SpecialAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;
};
