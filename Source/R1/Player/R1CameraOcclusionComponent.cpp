


#include "Player/R1CameraOcclusionComponent.h"
#include "Character/R1Character.h"
#include "Camera/CameraComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
UR1CameraOcclusionComponent::UR1CameraOcclusionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UR1CameraOcclusionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UR1CameraOcclusionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AR1Character* PlayerCharacter = Cast<AR1Character>(GetOwner());
	if (!PlayerCharacter) return;

	UCameraComponent* CameraComp = PlayerCharacter->FindComponentByClass<UCameraComponent>();
	if (!CameraComp) return;

	// 1. 카메라에서 플레이어 머리 쪽으로 레이저 발사 (발보다는 몸통/머리가 자연스러움)
	FVector StartLocation = CameraComp->GetComponentLocation();
	FVector EndLocation = PlayerCharacter->GetActorLocation() + FVector(0.f, 0.f, 50.f);

	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PlayerCharacter);

	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(CheckRadius);

	// SweepMultiByChannel 함수를 사용하여 CheckRadius 반경 내에 있는 모든 벽을 감지합니다.
	GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_Camera,
		CollisionShape,
		QueryParams
	);

	TSet<UPrimitiveComponent*> CurrentHitComponents;

	for (const FHitResult& Hit : HitResults)
	{
		UPrimitiveComponent* HitComp = Hit.GetComponent();
		if (HitComp)
		{
			CurrentHitComponents.Add(HitComp);

			// 아직 맵에 없는 녀석이면 새로 등록하고 타겟 투명도를 설정
			if (!OccludedComponentMap.Contains(HitComp))
			{
				InitializeComponentMIDs(HitComp);
			}
			OccludedComponentMap[HitComp].TargetOpacity = OccludedOpacity;
		}
	}

	// 3. 투명도 보간(Fading) 및 복구 로직
	for (auto It = OccludedComponentMap.CreateIterator(); It; ++It)
	{
		UPrimitiveComponent* Comp = It.Key();
		FOcclusionData& Data = It.Value();

		// 이번 프레임에 레이저에 안 맞았다면 다시 원래대로(1.0) 복구 준비
		if (!CurrentHitComponents.Contains(Comp))
		{
			Data.TargetOpacity = 1.0f;
		}

		// 투명도 서서히 변경 (FInterpTo)
		Data.CurrentOpacity = FMath::FInterpTo(Data.CurrentOpacity, Data.TargetOpacity, DeltaTime, FadeSpeed);

		// 실제 머티리얼 파라미터에 적용
		for (UMaterialInstanceDynamic* MID : Data.MIDs)
		{
			if (MID)
			{
				MID->SetScalarParameterValue(OpacityParamName, Data.CurrentOpacity);
			}
		}

		// 완전히 원래대로 돌아왔다면 맵에서 제거 (최적화)
		if (Data.TargetOpacity == 1.0f && FMath::IsNearlyEqual(Data.CurrentOpacity, 1.0f, 0.01f))
		{
			for (UMaterialInstanceDynamic* MID : Data.MIDs)
			{
				if (MID) MID->SetScalarParameterValue(OpacityParamName, 1.0f);
			}
			It.RemoveCurrent();
		}
	}
}

void UR1CameraOcclusionComponent::InitializeComponentMIDs(UPrimitiveComponent* TargetComp)
{
	FOcclusionData NewData;
	UMeshComponent* MeshComp = Cast<UMeshComponent>(TargetComp);

	if (MeshComp)
	{
		int32 NumMaterials = MeshComp->GetNumMaterials();
		for (int32 i = 0; i < NumMaterials; ++i)
		{
			UMaterialInterface* Mat = MeshComp->GetMaterial(i);
			UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Mat);

			if (!MID && Mat)
			{
				MID = MeshComp->CreateDynamicMaterialInstance(i, Mat);
			}

			if (MID)
			{
				NewData.MIDs.Add(MID);
			}
		}
	}

	OccludedComponentMap.Add(TargetComp, NewData);
}

