


#include "AI/BTService_FindTarget.h"
#include "Character/R1Player.h"
#include "Character/R1Monster.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/R1AIController.h"

UBTService_FindTarget::UBTService_FindTarget( )
{
	NodeName = TEXT("FindTargetService");
	Interval = 0.5f;
}

void UBTService_FindTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* LocalPawn = OwnerComp.GetAIOwner()->GetPawn();

	float SearchRadius = Cast<AR1Monster>(LocalPawn)->AggroRadius;

	if (LocalPawn == nullptr)
	{
		return;
	}

	UWorld* World = LocalPawn->GetWorld( );
	if (World == nullptr)
	{
		return;
	}

	
	FVector Location = LocalPawn->GetActorLocation();
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionQueryParam(NAME_None, false, LocalPawn);

	bool bResult = World->OverlapMultiByChannel(
		OverlapResults,
		Location,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(SearchRadius),
		CollisionQueryParam
	);

	if (bResult)
	{
		for (FOverlapResult& OverlapResult : OverlapResults)
		{
			AR1Player* R1Player = Cast<AR1Player>(OverlapResult.GetActor( ));
			if (R1Player)
			{
				if (R1Player->GetCreatureState() != ECreatureState::Dead)
				{
					OwnerComp.GetBlackboardComponent()->SetValueAsObject(TargetKey.SelectedKeyName, R1Player);
				}
				//DrawDebugSphere(World, Location, SearchRadius, 16, FColor::Green, false, 0.2f);
				return;
			}
		}
	}
	OwnerComp.GetBlackboardComponent( )->SetValueAsObject(TargetKey.SelectedKeyName, nullptr);
	//DrawDebugSphere(World, Location, SearchRadius, 16, FColor::Red, false, 0.2f);
}
