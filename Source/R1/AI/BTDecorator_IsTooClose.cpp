


#include "AI/BTDecorator_IsTooClose.h"
#include "AI/R1AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsTooClose::UBTDecorator_IsTooClose()
{
	NodeName = TEXT("Is Too Close");
}

bool UBTDecorator_IsTooClose::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(GetSelectedBlackboardKey()));

	if (!ControllingPawn || !Target) return false;

	// 거리가 세팅한 값보다 작거나 같으면 무조건 true!
	return ControllingPawn->GetDistanceTo(Target) <= TooCloseDistance;
}
