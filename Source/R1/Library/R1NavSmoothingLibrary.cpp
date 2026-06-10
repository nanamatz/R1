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

	// 2. Snap the click/target point to the nearest navigable point. Interactable
	//    centers (GetActorLocation) often sit slightly off the navmesh — raised, or
	//    inside the mesh — which makes pathfinding's end-point projection fail and the
	//    player not move at all. A generous extent lets those points find solid ground.
	//    Vertical is the loosest because object centers are usually above the floor.
	FVector TargetDest = Destination;
	FNavLocation ProjectedDest;
	const FVector ProjExtent(300.0f, 300.0f, 500.0f);
	if (NavSys->ProjectPointToNavigation(Destination, ProjectedDest, ProjExtent, &Pawn->GetNavAgentPropertiesRef()))
	{
		TargetDest = ProjectedDest.Location;
	}

	// 3. Query the path synchronously. Allow partial paths so a target on a
	//    disconnected navmesh island (or otherwise unreachable) still moves the player
	//    as far as the navmesh allows — ending near the target — instead of refusing
	//    to move. This is the core fix for clicking objects across navmesh gaps.
	FPathFindingQuery Query(Pawn, *NavData, Start, TargetDest);
	Query.SetAllowPartialPaths(true);
	FPathFindingResult Result = NavSys->FindPathSync(Query);

	// 4. Guard / fallback — no usable path at all.
	if (!Result.IsSuccessful() || !Result.Path.IsValid() || Result.Path->GetPathPoints().Num() < 2)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, TargetDest);
		return;
	}

	// 5. String-pull: greedy — from each anchor keep the farthest point reachable by a
	//    clear on-navmesh straight line, dropping the midpoints in between. Only worth
	//    doing when there are interior points to collapse (>= 3); a straight 2-point
	//    path (incl. a short partial path) is followed as-is.
	TArray<FNavPathPoint>& Pts = Result.Path->GetPathPoints();
	if (Pts.Num() >= 3)
	{
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
	}

	// 6. Issue the move through a PathFollowingComponent (get-or-create, mirroring
	//    what SimpleMoveToLocation does internally). Path already finished → no re-pathfind.
	UPathFollowingComponent* PFollowComp = Controller->FindComponentByClass<UPathFollowingComponent>();
	if (PFollowComp == nullptr)
	{
		PFollowComp = NewObject<UPathFollowingComponent>(Controller);
		PFollowComp->RegisterComponentWithWorld(World);
		PFollowComp->Initialize();
	}

	FAIMoveRequest MoveReq(TargetDest);
	MoveReq.SetUsePathfinding(false);

	FNavPathSharedPtr Path = Result.Path;
	PFollowComp->RequestMove(MoveReq, Path);
}
