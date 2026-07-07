#include "AI/BTTask_MoveToRange.h"
#include "AI/R1AIController.h"
#include "Character/R1Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/CapsuleComponent.h"

UBTTask_MoveToRange::UBTTask_MoveToRange()
{
	NodeName = TEXT("Move To Range");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveToRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AR1AIController* AIC = Cast<AR1AIController>(OwnerComp.GetAIOwner());
    if (!AIC) return EBTNodeResult::Failed;

    APawn* ControllingPawn = AIC->GetPawn();
    AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(BlackboardKey.SelectedKeyName));

    if (!ControllingPawn || !Target) return EBTNodeResult::Failed;

    // 사거리를 가져와서 멈출 거리를 계산합니다.
    AR1Character* SourceCharacter = Cast<AR1Character>(ControllingPawn);
    UR1AbilitySystemComponent* ASC = Cast<UR1AbilitySystemComponent>(SourceCharacter ? SourceCharacter->GetAbilitySystemComponent() : nullptr);

    float AttackRange = 150.0f;
    if (ASC)
    {
        AttackRange = ASC->GetNumericAttribute(UR1AttributeSet::GetAttackRangeAttribute());
    }

    float StopDistance = (AttackRange * RangeBuffer);
    StopDistance -= SourceCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();

    // 멈춤 거리를 AcceptanceRadius로 전달합니다.
    EPathFollowingRequestResult::Type Result = AIC->MoveToActor(Target, StopDistance, true, true, true, nullptr, true);

    if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::InProgress;
}

void UBTTask_MoveToRange::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    AR1AIController* AIC = Cast<AR1AIController>(OwnerComp.GetAIOwner());
    if (!AIC)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    APawn* ControllingPawn = AIC->GetPawn();
    AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(BlackboardKey.SelectedKeyName));

    if (!ControllingPawn || !Target)
    {
        AIC->StopMovement();
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    float CurrentDistance = ControllingPawn->GetDistanceTo(Target);
    
    // 이동 상태 확인
    EPathFollowingStatus::Type MoveStatus = AIC->GetMoveStatus();

    if (MoveStatus == EPathFollowingStatus::Idle)
    {
        AR1Character* SourceCharacter = Cast<AR1Character>(ControllingPawn);
        UR1AbilitySystemComponent* ASC = Cast<UR1AbilitySystemComponent>(SourceCharacter ? SourceCharacter->GetAbilitySystemComponent() : nullptr);

        float AttackRange = 150.0f;
        if (ASC)
        {
            AttackRange = ASC->GetNumericAttribute(UR1AttributeSet::GetAttackRangeAttribute());
        }

        // 멈춘 위치가 내 사거리(AttackRange) 안쪽이라면 성공한 것으로 쳐줍니다!
        if (CurrentDistance <= AttackRange)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
        else // 사거리도 안 닿았는데 멈춘 거라면 실패 처리!
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        }
    }
}
