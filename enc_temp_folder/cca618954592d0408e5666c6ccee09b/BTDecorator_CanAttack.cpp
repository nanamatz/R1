


#include "AI/BTDecorator_CanAttack.h"
#include "AI/R1AIController.h"
#include "Character/R1Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"

UBTDecorator_CanAttack::UBTDecorator_CanAttack()
{
	NodeName = TEXT("CanAttack");
}

bool UBTDecorator_CanAttack::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (ControllingPawn == nullptr)
	{
		return false;
	}

	AR1Character* Target = Cast<AR1Character>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetKey.SelectedKeyName));
	if (Target == nullptr)
	{
		return false;
	}

	AR1Character* SourceCharacter = Cast<AR1Character>(ControllingPawn);
	if (!SourceCharacter)
	{
		return false;
	}
	UAbilitySystemComponent* ASC = SourceCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}
	float AttackRange = ASC->GetNumericAttribute(UR1AttributeSet::GetAttackRangeAttribute());
	return (Target->GetDistanceTo(SourceCharacter) <= AttackRange);
}
