#include "AbilitySystem/Abilities/R1GameplayAbility_SummonAdds.h"
#include "R1LogChannels.h"
#include "R1Define.h"
#include "Character/R1Monster.h"
#include "Character/R1Boss.h"
#include "NavigationSystem.h"

UR1GameplayAbility_SummonAdds::UR1GameplayAbility_SummonAdds()
{
	// 액티베이션 사이에 SpawnedAdds를 유지해야 소환수 수를 추적할 수 있다.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

int32 UR1GameplayAbility_SummonAdds::PruneAndCountAlive()
{
	SpawnedAdds.RemoveAll([](const TWeakObjectPtr<AR1Monster>& Add)
	{
		return !Add.IsValid() || Add->IsActorBeingDestroyed() || Add->GetCreatureState() == ECreatureState::Dead;
	});

	return SpawnedAdds.Num();
}

bool UR1GameplayAbility_SummonAdds::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (AddClasses.Num() == 0)
	{
		return false;
	}

	// PruneAndCountAlive는 SpawnedAdds를 수정하므로 non-const다. UPROPERTY에 mutable을 쓸 수 없어
	// const_cast로 호출한다 (죽은 소환수를 걷어내는 것뿐이라 논리적으로는 const).
	return const_cast<UR1GameplayAbility_SummonAdds*>(this)->PruneAndCountAlive() < AliveCap;
}

void UR1GameplayAbility_SummonAdds::OnAttackEventReceived(FGameplayEventData Payload)
{
	AR1Boss* Boss = Cast<AR1Boss>(GetAvatarActorFromActorInfo());
	UWorld* World = GetWorld();

	if (!Boss || !World || AddClasses.Num() == 0)
	{
		return;
	}

	const int32 AliveNow = PruneAndCountAlive();
	const int32 Budget = FMath::Max(0, AliveCap - AliveNow);
	const int32 ToSpawn = FMath::Min(SpawnCount, Budget);

	if (ToSpawn <= 0)
	{
		return;
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	const FVector Origin = Boss->GetActorLocation();

	int32 SpawnedThisCast = 0;
	for (int32 i = 0; i < ToSpawn; ++i)
	{
		// 링 위에 균등 분배
		const float AngleDeg = (360.0f / ToSpawn) * i;
		const FVector Offset = FRotator(0.0f, AngleDeg, 0.0f).RotateVector(FVector(SpawnRingRadius, 0.0f, 0.0f));
		FVector DesiredLocation = Origin + Offset;

		// 네비메시에 투영. 실패하면 지오메트리 안에 소환하지 않고 건너뛴다.
		if (NavSys)
		{
			FNavLocation ProjectedLocation;
			if (!NavSys->ProjectPointToNavigation(DesiredLocation, ProjectedLocation, FVector(200.0f, 200.0f, 500.0f)))
			{
				UE_LOG(LogR1, Warning, TEXT("[SummonAdds] nav projection failed at %s, skipping"), *DesiredLocation.ToString());
				continue;
			}
			DesiredLocation = ProjectedLocation.Location;
		}

		const int32 ClassIndex = FMath::RandRange(0, AddClasses.Num() - 1);
		TSubclassOf<AR1Monster> AddClass = AddClasses[ClassIndex];
		if (!AddClass)
		{
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Boss;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		const FRotator SpawnRotation = (Origin - DesiredLocation).GetSafeNormal().Rotation();
		AR1Monster* Add = World->SpawnActor<AR1Monster>(AddClass, DesiredLocation, SpawnRotation, SpawnParams);

		if (Add)
		{
			// 스포너가 배치한 몬스터와 동일한 방 소속 처리를 받게 한다.
			Add->InitializeWithManager(Boss->OwningDungeonManager);
			SpawnedAdds.Add(Add);
			++SpawnedThisCast;
		}
	}

	UE_LOG(LogR1, Log, TEXT("SummonAdds: spawned %d adds (%d alive, cap %d)"), SpawnedThisCast, SpawnedAdds.Num(), AliveCap);
}
