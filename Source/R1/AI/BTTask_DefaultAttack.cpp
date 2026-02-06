


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
        FGameplayTagContainer TargetTags;
        TargetTags.AddTag(R1GameplayTags::Ability_Attack);

        if (ASC->TryActivateAbilitiesByTag(TargetTags))
        {
            // 2. 어빌리티가 끝날 때까지 기다리기 위해 델리게이트 연결
            // GAS의 OnAbilityEnded를 활용하거나, 간단하게 몬스터 클래스에 정의한 델리게이트 활용
            AR1Character* Monster = Cast<AR1Character>(AIC->GetPawn());
            Monster->OnAttackAbilityEnded.BindUObject(this, &UBTTask_Attack::OnAttackFinished, &OwnerComp);

            return EBTNodeResult::InProgress;
        }
    }
    return EBTNodeResult::Failed;
}
	AR1Monster* ControllingPawn = Cast<AR1Monster>(OwnerComp.GetAIOwner()->GetPawn());
	if (ControllingPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	ControllingPawn->SetCreatureState(ECreatureState::Skill);

	return EBTNodeResult::Type();
}
