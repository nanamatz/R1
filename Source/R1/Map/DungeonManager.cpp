


#include "Map/DungeonManager.h"
#include "Character/R1Character.h"
#include "Map/R1Door.h"
#include "Map/R1MonsterSpawner.h"
#include "EngineUtils.h"
#include "System/R1ObjectPoolSystem.h"
#include "Character/R1Monster.h"
#include "R1MapGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "Object/R1ItemActor.h"
#include "Data/R1ItemPoolData.h"
#include "Data/R1RoomDefinitionData.h"
#include "Math/UnrealMathUtility.h"
//#include "UObject/ConstructorHelpers.h" // 🌟 경로 탐색기 헤더 추가

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

	if (AActor* GeneratorActor = UGameplayStatics::GetActorOfClass(this, AR1MapGenerator::StaticClass()))
	{
		if (AR1MapGenerator* Generator = Cast<AR1MapGenerator>(GeneratorActor))
		{
			Generator->RegisterRoomManager(this, -1);
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
			Door->OpenDoor();
		}
	}
}

void ADungeonManager::CompleteRoom()
{
	if (bIsCleared) return;
	bIsCleared = true;

	UnlockRoomDoors();

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

	SpawnRoomClearReward();
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

void ADungeonManager::UnlockRoomDoorsInstantly()
{
	for (AR1Door* Door : RoomDoors)
	{
		if (IsValid(Door))
		{
			Door->SetLocked(false);
			Door->OpenDoorInstantly(); // 🌟 애니메이션 없이 즉시 열기!
		}
	}
}

void ADungeonManager::SpawnRoomClearReward()
{
	// 1. 방어 코드: 루팅 풀 에셋이 안 꽂혀있거나, 풀 안에 아이템이 0개면 패스!
	if (!RoomClearLootPool || RoomClearLootPool->DropItems.Num() == 0 || !ItemActorClass)
	{
		return;
	}

	if (ClearCondition == ER1RoomClearCondition::None)
	{
		return;
	}

	float DropRoll = FMath::FRandRange(1.f, 100.0f);
	if (DropRoll > RoomClearDropChance)
	{
		UE_LOG(LogTemp, Log, TEXT("방 클리어 보상 드랍 실패 (확률: %.1f%%, 결과: %.1f)"), RoomClearDropChance, DropRoll);
		return;
	}

	float TotalWeight = 0.0f;
	for (UR1ItemAssetData* ItemData : RoomClearLootPool->DropItems)
	{
		if (ItemData)
		{
			TotalWeight += ItemData->GetDropWeight(); // 🌟 아이템의 희귀도별 가중치 합산
		}
	}

	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float AccumulatedWeight = 0.0f;

	UR1ItemAssetData* SelectedItem = nullptr;

	for (UR1ItemAssetData* ItemData : RoomClearLootPool->DropItems)
	{
		if (!ItemData) continue;

		AccumulatedWeight += ItemData->GetDropWeight();
		if (RandomValue <= AccumulatedWeight)
		{
			SelectedItem = ItemData;
			break;
		}
	}

	if (!SelectedItem) return;

	// 3. 월드에 스폰!
	FVector SpawnLocation = GetActorLocation();
	SpawnLocation.Z = 50.0f;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AR1ItemActor* DroppedItem = GetWorld()->SpawnActor<AR1ItemActor>(ItemActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (DroppedItem)
	{
		// 🌟 희귀도 정보도 아이템 에셋이 들고 있으므로 그대로 던져줍니다.
		DroppedItem->InitItem(SelectedItem, SelectedItem->ItemRarity);

		UE_LOG(LogTemp, Warning, TEXT("방 클리어 보상 드랍! [%s] (가중치: %.0f)"),
			*SelectedItem->ItemName.ToString(), SelectedItem->GetDropWeight());
	}
}

void ADungeonManager::InitializeRoomData(UR1RoomDefinitionData* RoomData)
{
	if (!RoomData) return;

	// 🌟 1. 룸 데이터에서 정의된 보상 풀을 내 보상 풀로 덮어씌웁니다!
	RoomClearLootPool = RoomData->RoomClearLootPool;
	RoomClearDropChance = RoomData->RoomClearDropChance;

	// 2. (보너스) 방 타입에 따라 클리어 조건도 자동으로 맞춰줄 수 있습니다.
	if (RoomData->RoomType == ER1RoomContentType::Boss)
	{
		ClearCondition = ER1RoomClearCondition::KillBoss;
	}
	else if (RoomData->RoomType == ER1RoomContentType::Start)
	{
		ClearCondition = ER1RoomClearCondition::None;
	}
	else if (RoomData->RoomType == ER1RoomContentType::Treasure)
	{
		ClearCondition = ER1RoomClearCondition::Treasure;
	}
	else
	{
		ClearCondition = ER1RoomClearCondition::KillAllMonsters;
	}

	UE_LOG(LogTemp, Warning, TEXT("[DungeonManager] 방 데이터 주입 완료! 보상 풀: %s"),
		RoomClearLootPool ? *RoomClearLootPool->GetName() : TEXT("없음"));
}
