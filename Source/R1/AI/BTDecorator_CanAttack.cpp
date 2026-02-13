


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

	if (Target->GetDistanceTo(ControllingPawn) > AttackRange)
	{
		return false;
	}

	// [추가] 각도 체크 로직 (예: 정면 기준 좌우 60도, 총 120도 안에 있어야 공격 가능)
	FVector DirectionToTarget = (Target->GetActorLocation() - ControllingPawn->GetActorLocation()).GetSafeNormal();
	float DotResult = FVector::DotProduct(ControllingPawn->GetActorForwardVector(), DirectionToTarget);

	float AttackAngle = ASC->GetNumericAttribute(UR1AttributeSet::GetAttackAngleAttribute());

	float CosineThreshold = FMath::Cos(FMath::DegreesToRadians(AttackAngle / 2.f));

	if (DotResult <= CosineThreshold)
	{
		return false;
	}

	return true; // 거리도 가깝고, 내 앞쪽에 있을 때만 true!
}
