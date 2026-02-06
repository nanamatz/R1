


#include "AI/BTTask_DefaultAttack.h"
#include "R1AIController.h"
#include "Character/R1Monster.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_DefaultAttack::UBTTask_DefaultAttack()
{
	NodeName = TEXT("DefaultAttack");

}

EBTNodeResult::Type UBTTask_DefaultAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AR1Monster* ControllingPawn = Cast<AR1Monster>(OwnerComp.GetAIOwner()->GetPawn());
	if (ControllingPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	ControllingPawn->SetCreatureState(ECreatureState::Skill);

	return EBTNodeResult::Type();
}
