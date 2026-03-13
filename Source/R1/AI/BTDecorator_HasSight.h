

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_HasSight.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UBTDecorator_HasSight : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTDecorator_HasSight();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
