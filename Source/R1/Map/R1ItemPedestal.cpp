


#include "Map/R1ItemPedestal.h"
#include "Components/StaticMeshComponent.h"
#include "Data/R1ItemPoolData.h"
#include "Object/R1ItemActor.h"
#include "Math/UnrealMathUtility.h"

// Sets default values
AR1ItemPedestal::AR1ItemPedestal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PedestalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PedestalMesh"));
	RootComponent = PedestalMesh;

	// 제단 자체는 충돌 처리를 해서 플레이어가 뚫고 지나가지 못하게 막습니다.
	PedestalMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

// Called when the game starts or when spawned
void AR1ItemPedestal::BeginPlay()
{
	Super::BeginPlay();

	// 1. 방어 코드
	if (!TreasureLootPool || TreasureLootPool->DropItems.Num() == 0 || !ItemActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemPedestal] 루팅 풀이나 아이템 클래스가 세팅되지 않았습니다!"));
		return;
	}

	// 2. 가중치 기반 확률 계산
	float TotalWeight = 0.0f;
	for (UR1ItemAssetData* ItemData : TreasureLootPool->DropItems)
	{
		if (ItemData) TotalWeight += ItemData->GetDropWeight();
	}

	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float AccumulatedWeight = 0.0f;
	UR1ItemAssetData* SelectedItem = nullptr;

	for (UR1ItemAssetData* ItemData : TreasureLootPool->DropItems)
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

	// 3. 제단의 머리 위(Z축 +80 정도)에 아이템 스폰!
	FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 80.0f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AR1ItemActor* SpawnedItem = GetWorld()->SpawnActor<AR1ItemActor>(ItemActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (SpawnedItem)
	{
		// 데이터 덮어씌우기
		SpawnedItem->InitItem(SelectedItem, SelectedItem->ItemRarity);
	}
}

