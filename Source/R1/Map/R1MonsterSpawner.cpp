


#include "Map/R1MonsterSpawner.h"
#include "Character/R1Monster.h"
#include "Map/DungeonManager.h"
#include "System/R1ObjectPoolSystem.h"

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

	if (SpawnPresets.Num() > 0)
	{
		// 가중 랜덤으로 프리셋 하나 선택 (음수 가중치는 0으로 취급)
		float TotalWeight = 0.f;
		for (const FMonsterSpawnPreset& Preset : SpawnPresets)
		{
			TotalWeight += FMath::Max(Preset.Weight, 0.f);
		}

		int32 PickedIndex = 0;
		if (TotalWeight > 0.f)
		{
			float Roll = FMath::FRandRange(0.f, TotalWeight);
			for (int32 i = 0; i < SpawnPresets.Num(); ++i)
			{
				const float PresetWeight = FMath::Max(SpawnPresets[i].Weight, 0.f);
				if (PresetWeight <= 0.f) continue;

				Roll -= PresetWeight;
				if (Roll <= 0.f)
				{
					PickedIndex = i;
					break;
				}
			}
		}
		else
		{
			// 전 프리셋 가중치가 0이면 균등 랜덤
			PickedIndex = FMath::RandRange(0, SpawnPresets.Num() - 1);
		}

		SpawnFromList(SpawnPresets[PickedIndex].SpawnList);
	}
	else
	{
		// 레거시 경로: 프리셋 미사용 맵은 기존 SpawnList 그대로
		SpawnFromList(SpawnList);
	}
}

void AR1MonsterSpawner::SpawnFromList(const TArray<FMonsterSpawnData>& InSpawnList)
{
	UR1ObjectPoolSystem* Poolsystem = GetWorld()->GetGameInstance()->GetSubsystem<UR1ObjectPoolSystem>();
	if (!Poolsystem) return;

	for (const FMonsterSpawnData& SpawnData : InSpawnList)
	{
		if (!SpawnData.MonsterClass) continue;

		// 해당 몬스터에 설정된 '스폰 위치(드래그 위젯)'의 개수만큼 반복해서 소환합니다.
		for (const FVector& LocalPoint : SpawnData.SpawnPoints)
		{
			// 뷰포트에서 드래그한 상대 좌표(Local)를 진짜 월드 좌표(World)로 변환
			FVector WorldLocation = GetTransform().TransformPosition(LocalPoint);

			// 방향은 스포너가 바라보는 방향을 기준으로 생성
			FRotator WorldRotation = GetActorRotation();

			AR1Monster* SpawnedMonster = Poolsystem->GetMonster(SpawnData.MonsterClass, WorldLocation, WorldRotation);
			if (SpawnedMonster)
			{
				SpawnedMonster->InitializeWithManager(DungeonManager);
			}
		}
	}
}

