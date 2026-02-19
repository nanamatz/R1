


#include "Map/R1Door.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/R1Player.h"

// Sets default values
AR1Door::AR1Door()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(RootComp);
	DoorMesh->SetCollisionProfileName(TEXT("BlockAll")); // 기본적으로는 못 지나가게 막음

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);

	// 2. 트리거 박스 충돌 설정 (플레이어만 감지하도록)
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 50.0f)); // 적당한 크기로 조절

}

void AR1Door::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AR1Door::OnOverlapBegin);
	}
}

void AR1Door::SetupDoorConnection(int32 InTargetNodeID)
{
	TargetNodeID = InTargetNodeID;

	if (TargetNodeID == -1)
	{
		// 연결된 방이 없는 막다른 길이라면 문을 돌벽으로 바꾸거나 숨깁니다.
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
	else
	{
		// 연결된 방이 있다면 활성화
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		// TODO: 문이 활성화되었을 때의 시각적 효과(빛남 등) 추가 가능
	}
}

void AR1Door::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 막힌 문이면 무시
	if (bLocked || TargetNodeID == -1) return;

	// 부딪힌 액터가 플레이어 캐릭터인지 확인
	AR1Player* Player = Cast<AR1Player>(OtherActor);
	if (Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("[R1Door] 플레이어가 문에 닿았습니다! 향하는 방향: %d"), (int32)DoorDirection);

		// 델리게이트를 통해 외부(제너레이터 등)에 알림
		OnDoorEntered.Broadcast(DoorDirection);
	}
}

void AR1Door::SetLocked(bool bIsLocked)
{
	bLocked = bIsLocked;
	if (bLocked)
	{
		// 문을 잠그는 로직 (예: 충돌 활성화, 머티리얼 변경 등)
		//DoorMesh->SetCollisionProfileName(TEXT("BlockAll"));
		// TODO: 나중에 블루프린트에서 이 bLocked 변수를 읽고 '철창이 내려오는 애니메이션'을 재생하거나 '빨간색 빛'을 켤 수 있습니다.
	}
	else
	{
		// 문을 여는 로직 (예: 충돌 비활성화, 머티리얼 변경 등)
		//DoorMesh->SetCollisionProfileName(TEXT("NoCollision"));
		// TODO: 철창이 올라가거나 '초록색 빛'으로 변경
	}
}
