


#include "AI/BTTask_DefaultAttack.h"
#include "R1AIController.h"
#include "Character/R1Monster.h"
#include "R1GameplayTags.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_DefaultAttack::UBTTask_DefaultAttack()
{
	NodeName = TEXT("DefaultAttack");

}

EBTNodeResult::Type UBTTask_DefaultAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(AIC->GetPawn());

    if (ASI)
    {
        UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();

        // 1. 공격 어빌리티 실행 (Tag 기반)
        AR1Monster* Monster = Cast<AR1Monster>(AIC->GetPawn());

        Monster->ActivateAbility(R1GameplayTags::Ability_Attack);

        return EBTNodeResult::Succeeded;
    }
    return EBTNodeResult::Failed;
}
