


#include "AI/BTTask_SpecialAttack.h"
#include "R1AIController.h"
#include "Character/R1Monster.h"
#include "R1GameplayTags.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SpecialAttack::UBTTask_SpecialAttack()
{
	NodeName = TEXT("SpecialAttack");
}


EBTNodeResult::Type UBTTask_SpecialAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(AIC->GetPawn());
	if (ASI)
	{
		// 1. 공격 어빌리티 실행 (Tag 기반)
		AR1Monster* Monster = Cast<AR1Monster>(AIC->GetPawn());
		Monster->ActivateAbility(R1GameplayTags::Ability_Monster_SpecialAttack);
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}