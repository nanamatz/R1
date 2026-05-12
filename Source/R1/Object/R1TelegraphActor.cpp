#include "Object/R1TelegraphActor.h"
#include "Components/DecalComponent.h"
#include "Data/R1TelegraphData.h"
#include "Materials/MaterialInstanceDynamic.h"

AR1TelegraphActor::AR1TelegraphActor()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRoot);

	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	DecalComponent->SetupAttachment(RootComponent);
	
	// Default orientation for decals is looking down X, we want it looking down Z
	DecalComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
}

void AR1TelegraphActor::BeginPlay()
{
	Super::BeginPlay();
}

void AR1TelegraphActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TelegraphData && DecalMID)
	{
		ElapsedTime += DeltaTime;
		float FillAmount = FMath::Clamp(ElapsedTime / TelegraphData->Duration, 0.0f, 1.0f);
		DecalMID->SetScalarParameterValue(TEXT("FillAmount"), FillAmount);

		if (ElapsedTime >= TelegraphData->Duration)
		{
			Destroy();
		}
	}
}

void AR1TelegraphActor::InitializeTelegraph(UR1TelegraphData* InData)
{
	if (!InData) return;

	TelegraphData = InData;

	if (TelegraphData->DecalMaterial)
	{
		DecalMID = UMaterialInstanceDynamic::Create(TelegraphData->DecalMaterial, this);
		DecalComponent->SetDecalMaterial(DecalMID);
	}

	// Set size and offset based on shape
	switch (TelegraphData->Shape)
	{
	case ER1TelegraphShape::Circle:
		DecalComponent->DecalSize = FVector(100.0f, TelegraphData->TelegraphSize.X, TelegraphData->TelegraphSize.X);
		DecalComponent->SetRelativeLocation(FVector::ZeroVector);
		break;
	case ER1TelegraphShape::Rectangle:
		// X = Length (Forward), Y = Width (Side)
		DecalComponent->DecalSize = FVector(100.0f, TelegraphData->TelegraphSize.Y, TelegraphData->TelegraphSize.X);
		// Offset by Half-Length so the "start" of the decal is at the boss's feet
		DecalComponent->SetRelativeLocation(FVector(TelegraphData->TelegraphSize.X / 2.0f, 0.0f, 0.0f));
		break;
	case ER1TelegraphShape::Cone:
		DecalComponent->DecalSize = FVector(100.0f, TelegraphData->TelegraphSize.X, TelegraphData->TelegraphSize.X);
		DecalComponent->SetRelativeLocation(FVector(TelegraphData->TelegraphSize.X / 2.0f, 0.0f, 0.0f));
		break;
	}
}