


#include "AI/BTDecorator_IsTooClose.h"
#include "AI/R1AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/R1Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"

UBTDecorator_IsTooClose::UBTDecorator_IsTooClose()
{
	NodeName = TEXT("Is Too Close");
}

bool UBTDecorator_IsTooClose::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(GetSelectedBlackboardKey()));

	if (!ControllingPawn || !Target) return false;

	float FinalTooCloseDistance = TooCloseDistance;

	// 사거리를 가져와서 '너무 가깝다'고 판단하는 동적 로직 추가
	if (AR1Character* SourceCharacter = Cast<AR1Character>(ControllingPawn))
	{
		if (UAbilitySystemComponent* ASC = SourceCharacter->GetAbilitySystemComponent())
		{
			float AttackRange = ASC->GetNumericAttribute(UR1AttributeSet::GetAttackRangeAttribute());
			FinalTooCloseDistance = AttackRange * 0.3f;
		}
	}

	// 거리가 계산된 값보다 작거나 같으면 true!
	return ControllingPawn->GetDistanceTo(Target) <= FinalTooCloseDistance;
}
