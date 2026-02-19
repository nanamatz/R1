


#include "Map/R1MonsterSpawner.h"
#include "Character/R1Monster.h"
#include "Map/DungeonManager.h"

// Sets default values
AR1MonsterSpawner::AR1MonsterSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 아주 심플하게 루트 씬 컴포넌트 하나만 둡니다. (이제 화살표는 필요 없습니다)
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);
}

// Called when the game starts or when spawned
void AR1MonsterSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void AR1MonsterSpawner::SpawnMonster()
{
	if (!DungeonManager) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	for (const FMonsterSpawnData& SpawnData : SpawnList)
	{
		if (!SpawnData.MonsterClass) continue;

		// 2. 해당 몬스터에 설정된 '스폰 위치(드래그 위젯)'의 개수만큼 반복해서 소환합니다.
		for (const FVector& LocalPoint : SpawnData.SpawnPoints)
		{
			// [핵심] 뷰포트에서 드래그한 상대 좌표(Local)를 진짜 월드 좌표(World)로 변환!
			FVector WorldLocation = GetTransform().TransformPosition(LocalPoint);

			// 방향은 스포너가 바라보는 방향을 기준으로 생성
			FRotator WorldRotation = GetActorRotation();

			AR1Monster* SpawnedMonster = GetWorld()->SpawnActor<AR1Monster>(
				SpawnData.MonsterClass,
				WorldLocation,
				WorldRotation,
				SpawnParams
			);

			if (SpawnedMonster)
			{
				SpawnedMonster->InitializeWithManager(DungeonManager);
			}
		}
	}
}

