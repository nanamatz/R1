


#include "Player/R1CameraOcclusionComponent.h"
#include "Character/R1Character.h"
#include "Camera/CameraComponent.h"
#include "Components/MeshComponent.h"
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


// Called every frame
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
	// (기존에 세팅하신 커스텀 채널이 있다면 ECC_Camera 부분을 그 채널로 변경해 주세요)
	GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_Camera,
		CollisionShape,
		QueryParams
	);

	/*GetWorld()->LineTraceMultiByChannel(HitResults, StartLocation, EndLocation, ECC_Camera, QueryParams);*/

	// 이번 프레임에 레이저에 맞은 액터들을 담아둘 Set
	TSet<AActor*> CurrentHitActors;

	// 2. 맞은 액터들 검사
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			CurrentHitActors.Add(HitActor);

			// 아직 맵에 없는 녀석이면 새로 등록하고 타겟 투명도를 설정
			if (!OccludedActorMap.Contains(HitActor))
			{
				InitializeActorMIDs(HitActor);
			}
			OccludedActorMap[HitActor].TargetOpacity = OccludedOpacity;
		}
	}

	// 3. 투명도 보간(Fading) 및 복구 로직
	for (auto It = OccludedActorMap.CreateIterator(); It; ++It)
	{
		AActor* Actor = It.Key();
		FOcclusionData& Data = It.Value();

		// 이번 프레임에 레이저에 안 맞았다면 다시 원래대로(1.0) 복구 준비
		if (!CurrentHitActors.Contains(Actor))
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
			// 혹시 모를 오차를 위해 명시적으로 1.0 세팅
			for (UMaterialInstanceDynamic* MID : Data.MIDs)
			{
				if (MID) MID->SetScalarParameterValue(OpacityParamName, 1.0f);
			}
			It.RemoveCurrent();
		}
	}
}

void UR1CameraOcclusionComponent::InitializeActorMIDs(AActor* TargetActor)
{
	FOcclusionData NewData;

	// 액터가 가진 모든 메시 컴포넌트(스태틱, 스켈레탈 전부)를 찾습니다.
	TArray<UMeshComponent*> MeshComponents;
	TargetActor->GetComponents<UMeshComponent>(MeshComponents);

	for (UMeshComponent* MeshComp : MeshComponents)
	{
		if (!MeshComp) continue;

		int32 NumMaterials = MeshComp->GetNumMaterials();
		for (int32 i = 0; i < NumMaterials; ++i)
		{
			// 🌟 이미 MID가 만들어져 있다면 그걸 가져오고, 아니면 새로 만듭니다.
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

	OccludedActorMap.Add(TargetActor, NewData);
}

