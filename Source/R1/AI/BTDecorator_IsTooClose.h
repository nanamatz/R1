

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
	float TooCloseDistance = 150.0f; // 기본값을 낮춤 (사거리보다 확실히 작아야 함)

	UPROPERTY(EditAnywhere, Category = "AI")
	float SafeDistanceBuffer = 50.0f; // 도망친 후 멈출 때 여유분
};
