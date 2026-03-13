

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_IsTooClose.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UBTDecorator_IsTooClose : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTDecorator_IsTooClose();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	float TooCloseDistance = 300.0f; // 이 거리보다 가까우면 true 반환 (도망침)
};
