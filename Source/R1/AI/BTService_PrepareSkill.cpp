

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

	// 페이즈 전환 연출 중에는 새 스킬을 고르지 않는다.
	// 직전에 골라둔 값이 남아 있으면 ExecuteSkill이 그걸 그대로 발동해 연출을 덮어쓰므로
	// 반드시 지운다. (실제 발동 차단은 ASC의 ActivationInhibited가 담당한다.)
	if (BossCharacter->IsInPhaseTransition())
	{
		BlackboardComp->SetValueAsClass(BBKey_TargetAbilityClass.SelectedKeyName, nullptr);
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

	// 판정 결과를 블랙보드에 노출한다. 아래 조기 return들보다 먼저 기록해야
	// 스킬 후보가 없을 때도 BT가 최신 거리 상태를 볼 수 있다.
	if (BBKey_CanAttack.SelectedKeyName.IsNone() == false)
	{
		BlackboardComp->SetValueAsBool(BBKey_CanAttack.SelectedKeyName, bCanAttack);
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

	// 선택이 실제로 바뀐 경우에만 로그를 남긴다. 0.5초마다 같은 줄이 쌓이면
	// 정작 봐야 할 줄이 묻힌다.
	const UClass* PreviousClass = BlackboardComp->GetValueAsClass(BBKey_TargetAbilityClass.SelectedKeyName);
	if (PreviousClass != SelectedClass)
	{
		UE_LOG(LogR1, Log, TEXT("BTService_PrepareSkill: Selected Ability %s (CanAttack: %d, Candidates: %d)"), *SelectedClass->GetName(), bCanAttack, Candidates.Num());
	}

	BlackboardComp->SetValueAsClass(BBKey_TargetAbilityClass.SelectedKeyName, SelectedClass);
}

void UBTService_PrepareSkill::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	TickNode(OwnerComp, NodeMemory, 0.0f);
}
