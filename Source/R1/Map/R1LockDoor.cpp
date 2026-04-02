


#include "Map/R1LockDoor.h"
#include "Map/R1Door.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AR1LockDoor::AR1LockDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));

	LockDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockDoorMesh"));
	LockDoorMesh->SetupAttachment(RootComponent);
	LockDoorMesh->SetCollisionProfileName(TEXT("BlockAll"));

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);

	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Tags.Add(FName("Interactable"));
}

// Called when the game starts or when spawned
void AR1LockDoor::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(false);
}

void AR1LockDoor::SetLockVisibility(bool bIsVisible)
{
	if (bIsVisible)
	{
		LockDoorMesh->SetVisibility(true);
		LockDoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		LockDoorMesh->SetVisibility(false);
		LockDoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AR1LockDoor::OpenDoorSmoothly()
{
	TargetRotation = LockDoorMesh->GetRelativeRotation() + FRotator(0.f, 90.f, 0.f);
	LockDoorMesh->SetCollisionProfileName(TEXT("NoCollision"));
	bIsOpening = true;
	SetActorTickEnabled(true);
}

void AR1LockDoor::Highlight()
{
	if (LockDoorMesh && LockDoorMesh->IsVisible())
	{
		LockDoorMesh->SetRenderCustomDepth(true);
		LockDoorMesh->SetCustomDepthStencilValue(252);
	}
}

void AR1LockDoor::UnHighlight()
{
	if (LockDoorMesh) LockDoorMesh->SetRenderCustomDepth(false);
}

void AR1LockDoor::Interact_Implementation(AR1PlayerController* Interactor)
{
	if (LinkedMainDoor)
	{
		LinkedMainDoor->Interact_Implementation(Interactor);
	}
}

UPrimitiveComponent* AR1LockDoor::GetInteractTrigger()
{
	return TriggerBox;
}

// Called every frame
void AR1LockDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsOpening && LockDoorMesh)
	{
		FRotator CurrentRot = LockDoorMesh->GetRelativeRotation();
		FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRotation, DeltaTime, OpenSpeed);
		LockDoorMesh->SetRelativeRotation(NewRot);

		if (NewRot.Equals(TargetRotation, 1.0f))
		{
			bIsOpening = false;
			SetActorTickEnabled(false);
		}
	}
}

