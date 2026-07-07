


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

	// 1. 블랙보드에서 실행할 어빌리티 클래스 읽어오기
	UClass* TargetClass = BlackboardComp->GetValueAsClass(BBKey_TargetAbilityClass.SelectedKeyName);
	if (!TargetClass)
	{
		UE_LOG(LogR1, Warning, TEXT("BTTask_ExecuteSkill: TargetAbilityClass is NULL! (KeyName: %s)"), *BBKey_TargetAbilityClass.SelectedKeyName.ToString());
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogR1, Log, TEXT("BTTask_ExecuteSkill: Executing Ability %s"), *GetNameSafe(TargetClass));

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

	// 4. 해당 어빌리티 실행 시도
	if (FoundSpec && ASC->TryActivateAbility(FoundSpec->Handle))
	{
		return EBTNodeResult::InProgress;
	}

	// 실행 실패 시 바인딩 해제 후 종료
	ASC->AbilityEndedCallbacks.Remove(MyMemory->DelegateHandle);
	return EBTNodeResult::Failed;
}

void UBTTask_ExecuteSkill::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	FBTExecuteSkillMemory* MyMemory = reinterpret_cast<FBTExecuteSkillMemory*>(NodeMemory);

	// 5. 블랙보드 값 초기화 (서비스에서 다음 스킬을 고를 수 있게)
	if (UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent())
	{
		BlackboardComp->SetValueAsClass(BBKey_TargetAbilityClass.SelectedKeyName, nullptr);
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
