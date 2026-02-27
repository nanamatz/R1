


#include "Library/R1AbilitySystemLibrary.h"
#include "Engine/World.h"
#include "Character/R1Character.h"

TArray<AR1Character*> UR1AbilitySystemLibrary::GetChainLightningTargets(AR1Character* SourceActor, AR1Character* InitialTarget, float BounceRadius, int32 MaxBounces)
{
	TArray<AR1Character*> ResultTargets;

	// 예외 처리: 공격자나 첫 타겟이 없으면 빈 배열 반환
	if (!SourceActor || !InitialTarget || MaxBounces <= 0)
	{
		return ResultTargets;
	}

	UWorld* World = InitialTarget->GetWorld();
	if (!World) return ResultTargets;

	// 💡 1. 핑퐁 방지를 위한 '방문 목록(블랙리스트)'
	TArray<AR1Character*> VisitedActors;

	VisitedActors.Add(SourceActor);   // 내 캐릭터 제외
	//VisitedActors.Add(InitialTarget); // 첫 번째 맞은 애 제외

	AR1Character* CurrentCenterActor = InitialTarget;

	for (int32 i = 0; i < MaxBounces; ++i)
	{
		FVector CurrentOrigin = CurrentCenterActor->GetActorLocation();

		TArray<FOverlapResult> Overlaps;
		FCollisionQueryParams OverlapParams;

		for (AR1Character* VisitedChar : VisitedActors)
		{
			OverlapParams.AddIgnoredActor(VisitedChar);
		}

		// 몬스터가 폰(Pawn) 채널을 쓴다고 가정 (프로젝트에 맞게 변경 가능)
		World->OverlapMultiByObjectType(
			Overlaps,
			CurrentOrigin,
			FQuat::Identity,
			FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
			FCollisionShape::MakeSphere(BounceRadius),
			OverlapParams
		);

		AR1Character* ClosestValidTarget = nullptr;
		float MinDistanceSq = FMath::Square(BounceRadius + 1.0f); // 최소 거리 초기화

		// 💡 3. 찾은 몬스터들 중 '가장 가까우면서 벽에 막히지 않은' 놈 찾기
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AR1Character* PotentialTarget = Cast<AR1Character>(Overlap.GetActor());

			if (!PotentialTarget || PotentialTarget == CurrentCenterActor) continue;

			// 몬스터 체력이 0인지 확인
			if (PotentialTarget->GetCreatureState() == ECreatureState::Dead) continue;

			FVector TargetLoc = PotentialTarget->GetActorLocation();
			float DistSq = FVector::DistSquared(CurrentOrigin, TargetLoc);

			// 현재까지 찾은 가장 가까운 놈보다 더 가깝다면?
			if (DistSq < MinDistanceSq)
			{
				// 💡 4. 벽 관통 체크 (Line of Sight)
				FHitResult HitResult;
				FCollisionQueryParams LoSParams;
				LoSParams.AddIgnoredActor(CurrentCenterActor);
				LoSParams.AddIgnoredActor(PotentialTarget);

				// Visibility 채널로 레이캐스트를 쏴서 중간에 벽이 있는지 확인
				bool bHitWall = World->LineTraceSingleByChannel(
					HitResult,
					CurrentOrigin + FVector(0, 0, 50.0f),
					TargetLoc + FVector(0, 0, 50.0f),
					ECC_Visibility,
					LoSParams
				);

				// 벽에 막히지 않았다면! (최종 합격)
				if (!bHitWall)
				{
					ClosestValidTarget = PotentialTarget;
					MinDistanceSq = DistSq;
				}
			}
		}

		if (ClosestValidTarget)
		{
			ResultTargets.Add(ClosestValidTarget);
			VisitedActors.Add(ClosestValidTarget);

			CurrentCenterActor = ClosestValidTarget;
		}
		else
		{
			// 반경 내에 더 이상 조건에 맞는 타겟이 없으면 4마리를 못 채웠어도 조기 종료
			break;
		}
	}

	// 최종적으로 찾아낸 연쇄 타겟 1~3마리 반환
	return ResultTargets;
}
