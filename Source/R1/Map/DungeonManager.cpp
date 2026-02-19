


#include "Map/DungeonManager.h"
#include "Character/R1Character.h"
#include "Map/R1Door.h"
#include "Map/R1MonsterSpawner.h"
#include "DungeonManager.h"
#include "EngineUtils.h" // TActorIterator 용

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

	// 만약 조건이 'None(항상 열림)'으로 설정된 방(예: 0번 시작 방, 상점 방)이라면
	// 게임이 시작되자마자 즉시 문을 열어버리도록 무전을 칩니다.
	if (ClearCondition == ER1RoomClearCondition::None)
	{
		CompleteRoom();
	}
}

void ADungeonManager::RegisterMonster(AR1Character* MonsterToAdd)
{
	if (!IsValid(MonsterToAdd)) return;

	ActiveMonsters.Add(MonsterToAdd);
	UE_LOG(LogTemp, Log, TEXT("[RoomDirector] 몬스터 자동 등록됨: %s (현재 %d마리)"), *MonsterToAdd->GetName(), ActiveMonsters.Num());
}

void ADungeonManager::UnregisterMonster(AR1Character* DeadCharacter, AR1Character* Attacker)
{
	if (bIsCleared) return; // 이미 클리어된 방이면 무시

	if (ActiveMonsters.Contains(DeadCharacter))
	{
		ActiveMonsters.Remove(DeadCharacter);
		UE_LOG(LogTemp, Warning, TEXT("[DungeonManager] 몬스터 사망! 남은 수: %d"), ActiveMonsters.Num());

		// 명부가 텅 비었다면? 방 클리어!
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
			Door->SetLocked(true); // AR1Door에 만들어두신 SetLocked 함수 활용
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

	UE_LOG(LogTemp, Warning, TEXT("[DungeonManager] 빰빠밤! 방 클리어! 문 개방!"));

	// 1. 문 열어주기
	UnlockRoomDoors();

	// 2. [핵심 추가] 제너레이터에게 내 방 번호를 넘기며 클리어 사실을 통보!
	if (OnRoomCleared.IsBound())
	{
		OnRoomCleared.Broadcast(RoomNodeID);
	}

	// TODO: 여기에 보물상자를 스폰하거나, 플레이어에게 클리어 경험치를 주는 로직을 추가하면 완벽합니다!
}

void ADungeonManager::StartRoomCombat()
{
	if (bIsCleared)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DungeonManager] 이미 클리어된 방입니다. 평화롭게 지나가세요."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[DungeonManager] 미클리어 방 진입! 몬스터 스폰을 시작합니다."));

	// 2. 이 방에 널려있는 스포너들을 찾아서 스폰 명령을 내립니다!
	for (TActorIterator<AR1MonsterSpawner> It(GetWorld()); It; ++It)
	{
		// 스포이드로 나와 연결된 내 부하 스포너들만 작동시킵니다.
		if (It->DungeonManager == this)
		{
			It->SpawnMonster();
		}
	}
}
