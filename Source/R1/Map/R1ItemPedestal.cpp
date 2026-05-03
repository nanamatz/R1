


#include "Map/R1ItemPedestal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Data/R1ItemPoolData.h"
#include "Object/R1ItemActor.h"
#include "Math/UnrealMathUtility.h"
#include "Character/R1Player.h"

// Sets default values
AR1ItemPedestal::AR1ItemPedestal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	// 박스 크기 설정 (에디터에서 수정 가능) - 플레이어가 이 범위 안에 들어오면 작동
	TriggerBox->SetBoxExtent(FVector(300.f, 300.f, 150.f));

	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 플레이어(Pawn)만 감지

	PedestalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PedestalMesh"));
	PedestalMesh->SetupAttachment(RootComponent);
	PedestalMesh->SetCollisionProfileName(TEXT("BlockAll"));
	PedestalMesh->SetMobility(EComponentMobility::Movable);

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);
	BaseMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

// Called when the game starts or when spawned
void AR1ItemPedestal::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AR1ItemPedestal::OnOverlapBegin);
	}
	
}

void AR1ItemPedestal::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 1. 방어 코드
	if (!TreasureLootPool || TreasureLootPool->DropItems.Num() == 0 || !ItemActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemPedestal] 루팅 풀이나 아이템 클래스가 세팅되지 않았습니다!"));
		return;
	}

	if (bIsSpawned) return;

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
	FVector SpawnLocation = PedestalMesh->GetComponentLocation() + FVector(0.0f, 0.0f, 80.0f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AR1ItemActor* SpawnedItem = GetWorld()->SpawnActor<AR1ItemActor>(ItemActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (SpawnedItem)
	{
		SpawnedItem->InitItem(SelectedItem, SelectedItem->ItemRarity);
		SpawnedItem->PopEffect();
		bIsSpawned = true;
	}
}

