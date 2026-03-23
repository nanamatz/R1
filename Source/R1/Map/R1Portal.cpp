#include "Map/R1Portal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/R1Player.h"
#include "Map/R1MapGenerator.h"
#include "Player/R1PlayerController.h"

// Sets default values
AR1Portal::AR1Portal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalMesh->SetupAttachment(RootComp);
	PortalMesh->SetCollisionProfileName(TEXT("BlockAll")); // 기본적으로는 못 지나가게 막음

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);

	// 2. 트리거 박스 충돌 설정 (플레이어만 감지하도록)
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f)); // 적당한 크기로 조절
}

void AR1Portal::Interact_Implementation(AR1PlayerController* Interactor)
{
	if (!Interactor) return;

	// 바로 다음 층으로 이동!
	if (AR1MapGenerator* Generator = Cast<AR1MapGenerator>(UGameplayStatics::GetActorOfClass(this, AR1MapGenerator::StaticClass())))
	{
		Generator->GoToNextFloor();
		Destroy();
	}
}

// Called when the game starts or when spawned
void AR1Portal::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AR1Portal::OnOverlapBegin);
	}
}

void AR1Portal::Highlight()
{
	if (PortalMesh)
	{
		PortalMesh->SetRenderCustomDepth(true);
		PortalMesh->SetCustomDepthStencilValue(252); // 하이라이트 색상 설정 (프로젝트 설정에 따라 다름)
	}
}

void AR1Portal::UnHighlight()
{
	if (PortalMesh)
	{
		PortalMesh->SetRenderCustomDepth(false);
	}
}

UPrimitiveComponent* AR1Portal::GetInteractTrigger()
{
	return TriggerBox;
}




