


#include "AI/BTTask_ExecuteSkill.h"
#include "R1LogChannels.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AbilitySystemInterface.h"

UBTTask_ExecuteSkill::UBTTask_ExecuteSkill()
{
	NodeName = TEXT("Execute Boss Skill");
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_ExecuteSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTExecuteSkillMemory* MyMemory = reinterpret_cast<FBTExecuteSkillMemory*>(NodeMemory);
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!BlackboardComp || !AIController) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AIController->GetPawn();
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(ControlledPawn);

	if (!ASI || !ASI->GetAbilitySystemComponent()) return EBTNodeResult::Failed;

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();

	// 1. 실행할 어빌리티 클래스 결정.
	//    AbilityClassOverride가 지정돼 있으면 그걸 쓰고(BT에서 직접 지정한 스킬),
	//    아니면 기존대로 PrepareSkill이 골라둔 블랙보드 값을 읽는다.
	UClass* TargetClass = AbilityClassOverride;
	if (!TargetClass)
	{
		TargetClass = BlackboardComp->GetValueAsClass(BBKey_TargetAbilityClass.SelectedKeyName);
	}

	if (!TargetClass)
	{
		// PrepareSkill이 아직 후보를 못 골랐거나(전부 쿨다운) 페이즈 전환 중이면 정상적으로
		// 비어 있다. 매 틱 나올 수 있으므로 Warning이 아니라 Verbose.
		UE_LOG(LogR1, Verbose, TEXT("BTTask_ExecuteSkill: no ability selected yet (KeyName: %s)"), *BBKey_TargetAbilityClass.SelectedKeyName.ToString());
		return EBTNodeResult::Failed;
	}

	MyMemory->ExecutingClass = TargetClass;
	MyMemory->CachedOwnerComp = &OwnerComp;

	// 2. 어빌리티 종료 델리게이트 바인딩 (OwnerComp의 포인터를 캡처하여 안전하게 전달)
	UBehaviorTreeComponent* OwnerCompPtr = &OwnerComp;
	MyMemory->DelegateHandle = ASC->AbilityEndedCallbacks.AddLambda([this, OwnerCompPtr, TargetClass](UGameplayAbility* EndedAbility)
	{
		if (EndedAbility && EndedAbility->GetClass() == TargetClass)
		{
			// 스킬 완료 처리
			FinishLatentTask(*OwnerCompPtr, EBTNodeResult::Succeeded);
		}
	});

	// 3. ASC에 부여된 어빌리티를 순회하며 일치하는 클래스의 Handle 찾기
	FGameplayAbilitySpec* FoundSpec = nullptr;
	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetClass() == TargetClass)
		{
			FoundSpec = &Spec;
			break;
		}
	}

	// 4. 해당 어빌리티 실행 시도.
	//    로그는 '시도'가 아니라 '성공'에만 남긴다 — 시도 단계에서 찍으면 쿨다운에 막힌
	//    재시도까지 전부 발동한 것처럼 보여서 로그를 오독하게 된다.
	if (FoundSpec && ASC->TryActivateAbility(FoundSpec->Handle))
	{
		UE_LOG(LogR1, Log, TEXT("BTTask_ExecuteSkill: activated %s"), *GetNameSafe(TargetClass));
		return EBTNodeResult::InProgress;
	}

	// 실행 실패(쿨다운·코스트·차단태그, 또는 미부여). 매 틱 반복될 수 있으므로 Verbose.
	UE_LOG(LogR1, Verbose, TEXT("BTTask_ExecuteSkill: %s could not activate (%s)"),
		*GetNameSafe(TargetClass), FoundSpec ? TEXT("blocked") : TEXT("not granted to this ASC"));

	ASC->AbilityEndedCallbacks.Remove(MyMemory->DelegateHandle);
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_ExecuteSkill::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// BT가 이 분기를 선점(Lower Priority abort)하면 태스크만 끝나고 어빌리티는 계속
	// 돌아간다. 그러면 새 스킬의 몽타주가 이전 몽타주 위에 겹치므로 여기서 끊는다.
	FBTExecuteSkillMemory* MyMemory = reinterpret_cast<FBTExecuteSkillMemory*>(NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	IAbilitySystemInterface* ASI = AIController ? Cast<IAbilitySystemInterface>(AIController->GetPawn()) : nullptr;
	UAbilitySystemComponent* ASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;

	if (ASC && MyMemory->ExecutingClass.IsValid())
	{
		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.Ability->GetClass() == MyMemory->ExecutingClass.Get())
			{
				ASC->CancelAbilityHandle(Spec.Handle);
				break;
			}
		}
	}

	// 정리(델리게이트 해제·블랙보드)는 OnTaskFinished가 Aborted로도 호출되므로 거기서 처리된다.
	return EBTNodeResult::Aborted;
}

void UBTTask_ExecuteSkill::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	FBTExecuteSkillMemory* MyMemory = reinterpret_cast<FBTExecuteSkillMemory*>(NodeMemory);

	// 5. 블랙보드 값 초기화 (서비스에서 다음 스킬을 고를 수 있게).
	//    Override를 쓴 노드는 블랙보드를 읽지 않았으므로 건드리지 않는다 —
	//    PrepareSkill이 다른 노드용으로 방금 고른 값을 지워버리면 안 된다.
	if (AbilityClassOverride == nullptr)
	{
		if (UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent())
		{
			BlackboardComp->SetValueAsClass(BBKey_TargetAbilityClass.SelectedKeyName, nullptr);
		}
	}

	// 6. 태스크 종료 시 안전하게 찌꺼기 정리
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(AIController->GetPawn()))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				ASC->AbilityEndedCallbacks.Remove(MyMemory->DelegateHandle);
			}
		}
	}

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_ExecuteSkill::OnAbilityEnded(UGameplayAbility* EndedAbility)
{
}
