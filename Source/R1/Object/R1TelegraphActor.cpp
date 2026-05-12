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

	float DecalDepth = 500.0f;

	// Set size and offset based on shape
	switch (TelegraphData->Shape)
	{
		case ER1TelegraphShape::Circle:
		{
			// TelegraphSize.X 가 이미 반지름(Radius)이므로 그대로 사용합니다.
			DecalComponent->DecalSize = FVector(DecalDepth, TelegraphData->TelegraphSize.X, TelegraphData->TelegraphSize.X);
			DecalComponent->SetRelativeLocation(FVector::ZeroVector);
			break;
		}
		case ER1TelegraphShape::Rectangle:
		{
			// [수정 포인트 3] 직사각형: X=전체길이, Y=전체너비 라고 가정할 때, 
			// 데칼에는 절반(Half-Extents) 값으로 넣어야 크기가 정확히 맞습니다.
			float HalfLength = TelegraphData->TelegraphSize.X / 2.0f;
			float HalfWidth = TelegraphData->TelegraphSize.Y / 2.0f;

			// 주의: DecalSize 구조는 FVector(깊이, 너비, 길이) 순서로 적용됩니다.
			DecalComponent->DecalSize = FVector(DecalDepth, HalfWidth, HalfLength);

			// 절반의 길이만큼 앞으로 밀어주면 보스 발끝에서부터 장판이 시작됩니다.
			DecalComponent->SetRelativeLocation(FVector(HalfLength, 0.0f, 0.0f));
			break;
		}

		case ER1TelegraphShape::Cone:
		{
			// 부채꼴도 마찬가지로 전체 길이를 절반으로 나누어 넣어야 합니다.
			float HalfLength = TelegraphData->TelegraphSize.X / 2.0f;
			DecalComponent->DecalSize = FVector(DecalDepth, HalfLength, HalfLength);
			DecalComponent->SetRelativeLocation(FVector(HalfLength, 0.0f, 0.0f));
			break;
		}
	}
}