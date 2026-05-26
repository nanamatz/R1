#include "Map/R1PlayerSpawnMarker.h"
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"

AR1PlayerSpawnMarker::AR1PlayerSpawnMarker()
{
	PrimaryActorTick.bCanEverTick = false;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->ArrowSize = 2.0f;
	ArrowComponent->ArrowColor = FColor::Green;
	SetRootComponent(ArrowComponent);

	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillboardComponent"));
	BillboardComponent->SetupAttachment(ArrowComponent);
	BillboardComponent->bIsScreenSizeScaled = true;
}
