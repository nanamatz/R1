

#include "AI/BTService_PrepareSkill.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "AbilitySystem/Attribute/MonsterAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "Character/R1Boss.h"

UBTService_PrepareSkill::UBTService_PrepareSkill()
{
	NodeName = TEXT("Prepare Boss Skill");
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UBTService_PrepareSkill::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (NodeMemory != nullptr)
	{
		Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!BlackboardComp || !AIC) return;

	AR1Boss* BossCharacter = Cast<AR1Boss>(AIC->GetPawn());
	if (!BossCharacter) return;

	// 1. 이미 선택된 어빌리티가 있다면 스킵
	UClass* CurrentTargetClass = BlackboardComp->GetValueAsClass(BBKey_TargetAbilityClass.SelectedKeyName);
	if (CurrentTargetClass) return;

	// =========================================================
	// 2. 근접 공격 가능 여부(CanAttack) 직접 계산
	// =========================================================
	bool bCanAttack = false;
	AR1Character* Target = Cast<AR1Character>(BlackboardComp->GetValueAsObject(BBKey_TargetActor.SelectedKeyName));
	UAbilitySystemComponent* ASC = BossCharacter->GetAbilitySystemComponent();

	if (Target && ASC)
	{
		float AttackRange = ASC->GetNumericAttribute(UR1AttributeSet::GetAttackRangeAttribute());
		float Distance = Target->GetDistanceTo(BossCharacter);

		Distance -= BossCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
		Distance -= Target->GetCapsuleComponent()->GetScaledCapsuleRadius();

		// 거리가 사거리 안쪽일 때만 각도 계산
		if (Distance <= AttackRange)
		{
			FVector DirectionToTarget = (Target->GetActorLocation() - BossCharacter->GetActorLocation()).GetSafeNormal();
			float DotResult = FVector::DotProduct(BossCharacter->GetActorForwardVector(), DirectionToTarget);
			float AttackAngle = ASC->GetNumericAttribute(UMonsterAttributeSet::GetAttackAngleAttribute());
			float CosineThreshold = FMath::Cos(FMath::DegreesToRadians(AttackAngle / 2.f));

			if (DotResult > CosineThreshold)
			{
				bCanAttack = true; // 거리도 가깝고, 각도도 맞음!
			}
		}
	}

	// =========================================================
	// 3. 조건에 맞는 어빌리티 배열 가져와서 랜덤 뽑기
	// =========================================================
	TArray<TSubclassOf<UGameplayAbility>> AbilityList = bCanAttack ? BossCharacter->GetDefaultSkillList() : BossCharacter->GetAdditionalSkillList();

	if (AbilityList.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, AbilityList.Num() - 1);
		UClass* SelectedClass = AbilityList[RandomIndex];

		if (SelectedClass)
		{
			BlackboardComp->SetValueAsClass(BBKey_TargetAbilityClass.SelectedKeyName, SelectedClass);
			UE_LOG(LogTemp, Log, TEXT("BTService_PrepareSkill: Selected Ability %s (CanAttack: %d)"), *SelectedClass->GetName(), bCanAttack);
		}
	}
}

void UBTService_PrepareSkill::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	TickNode(OwnerComp, NodeMemory, 0.0f);
}
