#include "AI/BTTask_MoveToRange.h"
#include "AIController.h"
#include "Character/R1Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

UBTTask_MoveToRange::UBTTask_MoveToRange()
{
	NodeName = TEXT("Move To Range");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveToRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return EBTNodeResult::Failed;

    APawn* ControllingPawn = AIC->GetPawn();
    AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(BlackboardKey.SelectedKeyName));

    if (!ControllingPawn || !Target) return EBTNodeResult::Failed;

    // Start moving
    AIC->MoveToActor(Target, -1.0f, true, true, true, nullptr, true);

    return EBTNodeResult::InProgress;
}

void UBTTask_MoveToRange::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIC = OwnerComp.GetAIOwner();
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

    AR1Character* SourceCharacter = Cast<AR1Character>(ControllingPawn);
    UAbilitySystemComponent* ASC = SourceCharacter ? SourceCharacter->GetAbilitySystemComponent() : nullptr;

    float AttackRange = 150.0f;
    if (ASC)
    {
        AttackRange = ASC->GetNumericAttribute(UR1AttributeSet::GetAttackRangeAttribute());
    }

    float StopDistance = FMath::Max(50.0f, AttackRange * RangeBuffer);

    float CurrentDistance = ControllingPawn->GetDistanceTo(Target);
    if (CurrentDistance <= StopDistance)
    {
        AIC->StopMovement();
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
    else
    {
        // Optionally update the destination if the target moved significantly
        // For simplicity, we assume MoveToActor is handling the target location update
    }
}
