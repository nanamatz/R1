#include "Library/R1NavSmoothingLibrary.h"

#include "NavigationSystem.h"
#include "NavigationData.h"
#include "NavigationPath.h"
#include "AI/Navigation/NavigationTypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

void UR1NavSmoothingLibrary::SmoothMoveTo(AController* Controller, const FVector& Destination)
{
	// 1. Validate inputs — fail safe (no crash, no movement) on anything missing.
	if (Controller == nullptr)
	{
		return;
	}

	APawn* Pawn = Controller->GetPawn();
	UWorld* World = Controller->GetWorld();
	if (Pawn == nullptr || World == nullptr)
	{
		return;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (NavSys == nullptr)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, Destination);
		return;
	}

	const FVector Start = Pawn->GetNavAgentLocation();
	const ANavigationData* NavData = NavSys->GetNavDataForProps(Pawn->GetNavAgentPropertiesRef(), Start);
	if (NavData == nullptr)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, Destination);
		return;
	}

	// 2. Query the path synchronously.
	FPathFindingQuery Query(Pawn, *NavData, Start, Destination);
	FPathFindingResult Result = NavSys->FindPathSync(Query);

	// 3. Guard / fallback — invalid or too short to smooth (straight shot).
	if (!Result.IsSuccessful() || !Result.Path.IsValid() || Result.Path->GetPathPoints().Num() < 3)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, Destination);
		return;
	}

	// 4. String-pull: greedy — from each anchor keep the farthest point reachable by a
	//    clear on-navmesh straight line, dropping the midpoints in between.
	TArray<FNavPathPoint>& Pts = Result.Path->GetPathPoints();
	TArray<FNavPathPoint> Kept;
	Kept.Reserve(Pts.Num());
	Kept.Add(Pts[0]);

	int32 Anchor = 0;
	while (Anchor < Pts.Num() - 1)
	{
		int32 Farthest = Anchor + 1; // the immediate next point is always reachable
		for (int32 i = Anchor + 2; i < Pts.Num(); ++i)
		{
			FVector HitLocation;
			const bool bBlocked = UNavigationSystemV1::NavigationRaycast(
				Controller, Pts[Anchor].Location, Pts[i].Location, HitLocation, nullptr, Controller);

			if (bBlocked)
			{
				break; // can't see point i directly — stop extending from this anchor
			}
			Farthest = i; // clear straight line on navmesh — we can go straight to i
		}

		Kept.Add(Pts[Farthest]);
		Anchor = Farthest;
	}

	Pts = MoveTemp(Kept); // overwrite the path with the smoothed point list

	// 5. Issue the move through a PathFollowingComponent (get-or-create, mirroring
	//    what SimpleMoveToLocation does internally). Path already finished → no re-pathfind.
	UPathFollowingComponent* PFollowComp = Controller->FindComponentByClass<UPathFollowingComponent>();
	if (PFollowComp == nullptr)
	{
		PFollowComp = NewObject<UPathFollowingComponent>(Controller);
		PFollowComp->RegisterComponentWithWorld(World);
		PFollowComp->Initialize();
	}

	FAIMoveRequest MoveReq(Destination);
	MoveReq.SetUsePathfinding(false);

	FNavPathSharedPtr Path = Result.Path;
	PFollowComp->RequestMove(MoveReq, Path);
}
