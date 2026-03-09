


#include "Map/DungeonManager.h"
#include "Character/R1Character.h"
#include "Map/R1Door.h"
#include "Map/R1MonsterSpawner.h"
#include "EngineUtils.h"
#include "System/R1ObjectPoolSystem.h"
#include "Character/R1Monster.h"
#include "R1MapGenerator.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ADungeonManager::ADungeonManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ADungeonManager::BeginPlay()
{
	Super::BeginPlay();

	if (ClearCondition == ER1RoomClearCondition::None)
	{
		CompleteRoom();
	}

	if (AActor* GeneratorActor = UGameplayStatics::GetActorOfClass(this, AR1MapGenerator::StaticClass()))
	{
		if (AR1MapGenerator* Generator = Cast<AR1MapGenerator>(GeneratorActor))
		{
			Generator->RegisterRoomManager(this);
		}
	}
}

void ADungeonManager::RegisterMonster(AR1Character* MonsterToAdd)
{
	if (!IsValid(MonsterToAdd)) return;

	ActiveMonsters.Add(MonsterToAdd);
	AR1Monster* R1Monster = Cast<AR1Monster>(MonsterToAdd);
	if (R1Monster)
	{
		R1Monster->OnReadyToSleep.RemoveDynamic(this, &ADungeonManager::HandleMonsterReadyToSleep);
		R1Monster->OnReadyToSleep.AddDynamic(this, &ADungeonManager::HandleMonsterReadyToSleep);
	}
}

void ADungeonManager::UnregisterMonster(AR1Character* DeadCharacter, AR1Character* Attacker)
{
	if (bIsCleared) return;

	if (ActiveMonsters.Contains(DeadCharacter))
	{
		ActiveMonsters.Remove(DeadCharacter);

		if (ActiveMonsters.Num() == 0)
		{
			CompleteRoom();
		}
	}
}


void ADungeonManager::LockRoomDoors()
{
	for (AR1Door* Door : RoomDoors)
	{
		if (IsValid(Door))
		{
			Door->SetLocked(true);
		}
	}
}

void ADungeonManager::UnlockRoomDoors()
{
	for (AR1Door* Door : RoomDoors)
	{
		if (IsValid(Door))
		{
			Door->SetLocked(false);
		}
	}
}

void ADungeonManager::CompleteRoom()
{
	if (bIsCleared) return;
	bIsCleared = true;

	UnlockRoomDoors();

	// 🌟 [추가] 보스 방이고, 포탈 클래스가 세팅되어 있다면 방 중앙에 소환!
	if (ClearCondition == ER1RoomClearCondition::KillBoss)
	{
		bool bIsLastFloor = false;

		if (AR1MapGenerator* Generator = Cast<AR1MapGenerator>(UGameplayStatics::GetActorOfClass(this, AR1MapGenerator::StaticClass())))
		{
			bIsLastFloor = Generator->IsLastFloor();

			if (bIsLastFloor)
			{
				// TODO: 여기에 게임 클리어 UI를 띄우거나, 엔딩 씬으로 넘어가는 코드
			}
			else if (PortalClass)
			{
				FVector PortalLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
				GetWorld()->SpawnActor<AActor>(PortalClass, PortalLocation, FRotator::ZeroRotator);
			}
		}
	}

	if (OnRoomCleared.IsBound())
	{
		OnRoomCleared.Broadcast(RoomNodeID);
	}

	// TODO 보상 지급 로직
}

void ADungeonManager::StartRoomCombat()
{
	if (bIsCleared)
	{
		return;
	}

	for (TActorIterator<AR1MonsterSpawner> It(GetWorld()); It; ++It)
	{
		if (It->DungeonManager == this)
		{
			It->SpawnMonster();
		}
	}
}

void ADungeonManager::HandleMonsterReadyToSleep(AR1Monster* DeadMonster)
{
	if (!IsValid(DeadMonster)) return;

	DeadMonster->OnReadyToSleep.RemoveDynamic(this, &ADungeonManager::HandleMonsterReadyToSleep);

	UnregisterMonster(DeadMonster, nullptr);

	// 1. 게임 인스턴스에서 오브젝트 풀 서브시스템을 찾습니다.
	if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		UR1ObjectPoolSystem* PoolSubsystem = GameInstance->GetSubsystem<UR1ObjectPoolSystem>();
		if (PoolSubsystem)
		{
			PoolSubsystem->ReturnMonster(DeadMonster);

			UE_LOG(LogTemp, Log, TEXT("[DungeonManager] 몬스터를 성공적으로 창고에 반환했습니다: %s"), *DeadMonster->GetName());
		}
	}
}
