


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
	PrimaryActorTick.bCanEverTick = true;
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

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);
	BaseMesh->SetCollisionProfileName(TEXT("BlockAll"));


}

// Called when the game starts or when spawned
void AR1ItemPedestal::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(false);

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AR1ItemPedestal::OnOverlapBegin);
	}

	if (PedestalMesh)
	{
		// 1. 목표 지점은 현재 위치에서 Z로 80만큼 위
		TargetRelativeLocation = PedestalMesh->GetRelativeLocation();

		// 2. 시작 지점은 Z로 80만큼 아래 (바닥에 숨김)
		FVector StartLocation = TargetRelativeLocation - FVector(0.f, 0.f, 80.f);
		PedestalMesh->SetRelativeLocation(StartLocation);
	}
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
	FVector SpawnLocation = PedestalMesh->GetComponentLocation() + FVector(0.0f, 0.0f, 80.0f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AR1ItemActor* SpawnedItem = GetWorld()->SpawnActor<AR1ItemActor>(ItemActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (SpawnedItem)
	{
		// 데이터 덮어씌우기
		SpawnedItem->InitItem(SelectedItem, SelectedItem->ItemRarity);
		SpawnedItem->AttachToComponent(PedestalMesh, FAttachmentTransformRules::KeepWorldTransform);
	}
}

void AR1ItemPedestal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsRising || !PedestalMesh) return;

	// 1. 위치 보간 (VInterpTo)
	FVector CurrentLoc = PedestalMesh->GetRelativeLocation();
	FVector NewLoc = FMath::VInterpTo(CurrentLoc, TargetRelativeLocation, DeltaTime, RiseSpeed);
	PedestalMesh->SetRelativeLocation(NewLoc);

	// 2. 지속적인 회전 (Yaw)
	// 회전은 목표치 없이 계속 도는 것이 자연스러우므로 AddRelativeRotation 사용
	PedestalMesh->AddRelativeRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));

	// 3. 목표 높이에 거의 도달했는지 확인
	if (NewLoc.Equals(TargetRelativeLocation, 0.5f))
	{
		// 위치는 고정시키고 상승 상태 종료
		PedestalMesh->SetRelativeLocation(TargetRelativeLocation);

		 bIsRising = false; 
		 SetActorTickEnabled(false); 
	}
}

void AR1ItemPedestal::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 1. 이미 한 번 작동했다면 무시 (일회성 보장)
	if (bHasRisen) return;

	if (OtherActor)
	{
		bHasRisen = true; // 이제 두 번 다시 작동하지 않음
		StartRising();

		// 3. (최적화) 더 이상 트리거가 감지할 필요 없으므로 콜리전 완전히 끄기
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		UE_LOG(LogTemp, Warning, TEXT("플레이어 접근 감지: 제단 상승 시작!"));
	}
}

void AR1ItemPedestal::StartRising()
{
	if (bIsRising) return;

	bIsRising = true;
	SetActorTickEnabled(true);
}

