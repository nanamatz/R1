#include "AI/BTTask_ActivateAbilityByTag.h"
#include "AIController.h"
#include "Character/R1Monster.h"

EBTNodeResult::Type UBTTask_ActivateAbilityByTag::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	AR1Monster* Monster = AIC ? Cast<AR1Monster>(AIC->GetPawn()) : nullptr;

	if (Monster)
	{
		// 공격 어빌리티 실행 (Tag 기반)
		Monster->ActivateAbility(AbilityTag);
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
