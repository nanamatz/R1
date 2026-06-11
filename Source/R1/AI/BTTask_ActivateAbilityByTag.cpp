#include "AI/BTTask_ActivateAbilityByTag.h"
#include "AIController.h"
#include "AbilitySystemInterface.h"
#include "Character/R1Monster.h"

EBTNodeResult::Type UBTTask_ActivateAbilityByTag::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(AIC->GetPawn());

	if (ASI)
	{
		// 1. 공격 어빌리티 실행 (Tag 기반)
		AR1Monster* Monster = Cast<AR1Monster>(AIC->GetPawn());
		Monster->ActivateAbility(AbilityTag);
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
