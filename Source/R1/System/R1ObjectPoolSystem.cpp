


#include "System/R1ObjectPoolSystem.h"
#include "Character/R1Monster.h"


AR1Monster* UR1ObjectPoolSystem::GetMonster(TSubclassOf<AR1Monster> MonsterClass, FVector Location, FRotator Rotation)
{
	if (!MonsterClass) return nullptr;

	// 1. 창고에 해당 몬스터 클래스의 대기실이 있고, 쉬고 있는 몬스터가 있다면?
	if (MonsterPools.Contains(MonsterClass) && MonsterPools[MonsterClass].Pool.Num() > 0)
	{
		// 대기실에서 한 마리를 꺼냅니다. (Pop)
		AR1Monster* PooledMonster = MonsterPools[MonsterClass].Pool.Pop();

		if (IsValid(PooledMonster))
		{
			// 원하는 위치와 방향으로 이동시킨 뒤 출근(활성화) 시킵니다.
			PooledMonster->SetActorLocationAndRotation(Location, Rotation);
			PooledMonster->WakeUp(); // AR1Monster에 구현해둘 함수
			return PooledMonster;
		}
	}

	// 2. 창고가 비어있다면, 어쩔 수 없이 새로 생성(Spawn)해서 줍니다.
	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AR1Monster* NewMonster = World->SpawnActor<AR1Monster>(MonsterClass, Location, Rotation, SpawnParams);
		return NewMonster; // 새로 태어난 몬스터는 BeginPlay를 거치므로 WakeUp이 당장 필요 없을 수 있습니다.
	}

	return nullptr;
}

void UR1ObjectPoolSystem::ReturnMonster(AR1Monster* ReturningMonster)
{
	if (!IsValid(ReturningMonster)) return;

	ReturningMonster->GoToSleep(); // AR1Monster에 구현해둘 함수

	TSubclassOf<AR1Monster> ClassType = ReturningMonster->GetClass();
	MonsterPools.FindOrAdd(ClassType).Pool.Add(ReturningMonster);
}
