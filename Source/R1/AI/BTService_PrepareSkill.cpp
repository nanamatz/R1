

#include "AI/BTService_PrepareSkill.h"
#include "R1LogChannels.h"
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

	// 페이즈 전환 연출 중에는 새 스킬을 고르지 않는다. 블랙보드를 건드리지 않으므로
	// ExecuteSkill이 null을 읽고 Failed를 반환하고, 연출이 끝나면 자연히 재개된다.
	if (BossCharacter->IsInPhaseTransition())
	{
		return;
	}

	bool bCanAttack = false;
	AR1Character* Target = Cast<AR1Character>(BlackboardComp->GetValueAsObject(BBKey_TargetActor.SelectedKeyName));
	UAbilitySystemComponent* ASC = BossCharacter->GetAbilitySystemComponent();

	if (Target && ASC)
	{
		float AttackRange = ASC->GetNumericAttribute(UR1AttributeSet::GetAttackRangeAttribute()) * DistanceMargin;
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

	TArray<TSubclassOf<UGameplayAbility>> AbilityList = bCanAttack ? BossCharacter->GetDefaultSkillList() : BossCharacter->GetAdditionalSkillList();

	if (AbilityList.Num() == 0 || ASC == nullptr)
	{
		return;
	}

	// 쿨다운/코스트/차단태그를 통과하는 어빌리티만 후보로 남긴다.
	// CanActivateAbility 한 번으로 세 가지가 모두 검사된다.
	TArray<UClass*> Candidates;
	Candidates.Reserve(AbilityList.Num());

	const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability == nullptr || !AbilityList.Contains(Spec.Ability->GetClass()))
		{
			continue;
		}

		if (ActorInfo && Spec.Ability->CanActivateAbility(Spec.Handle, ActorInfo))
		{
			Candidates.Add(Spec.Ability->GetClass());
		}
	}

	// 후보가 하나도 없으면(전부 쿨다운) 블랙보드를 건드리지 않는다.
	// ExecuteSkill이 null을 읽고 Failed를 반환하며, BT가 다음 틱에 다시 시도한다.
	if (Candidates.Num() == 0)
	{
		UE_LOG(LogR1, Verbose, TEXT("BTService_PrepareSkill: no ability off cooldown (CanAttack: %d)"), bCanAttack);
		return;
	}

	UClass* SelectedClass = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
	BlackboardComp->SetValueAsClass(BBKey_TargetAbilityClass.SelectedKeyName, SelectedClass);
	UE_LOG(LogR1, Log, TEXT("BTService_PrepareSkill: Selected Ability %s (CanAttack: %d, Candidates: %d)"), *SelectedClass->GetName(), bCanAttack, Candidates.Num());
}

void UBTService_PrepareSkill::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	TickNode(OwnerComp, NodeMemory, 0.0f);
}
