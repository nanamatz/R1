


#include "AI/BTDecorator_CanAttack.h"
#include "AI/R1AIController.h"
#include "Character/R1Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "AbilitySystem/Attribute/MonsterAttributeSet.h"
#include "Components/CapsuleComponent.h"

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

	// 캡슐 반지름을 고려한 정확한 거리 계산
	float Distance = Target->GetDistanceTo(ControllingPawn);
	
	if (ACharacter* MyChar = Cast<ACharacter>(ControllingPawn))
	{
		Distance -= MyChar->GetCapsuleComponent()->GetScaledCapsuleRadius();
	}
	if (ACharacter* TargetChar = Cast<ACharacter>(Target))
	{
		Distance -= TargetChar->GetCapsuleComponent()->GetScaledCapsuleRadius();
	}

	if (Distance > AttackRange)
	{
		return false;
	}

	FVector DirectionToTarget = (Target->GetActorLocation() - ControllingPawn->GetActorLocation()).GetSafeNormal();
	float DotResult = FVector::DotProduct(ControllingPawn->GetActorForwardVector(), DirectionToTarget);

	float AttackAngle = ASC->GetNumericAttribute(UMonsterAttributeSet::GetAttackAngleAttribute());

	float CosineThreshold = FMath::Cos(FMath::DegreesToRadians(AttackAngle / 2.f));

	if (DotResult <= CosineThreshold)
	{
		return false;
	}

	return true; // 거리도 가깝고, 내 앞쪽에 있을 때만 true!
}
