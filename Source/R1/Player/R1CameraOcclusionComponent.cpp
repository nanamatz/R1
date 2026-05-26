
#include "Player/R1CameraOcclusionComponent.h"
#include "Character/R1Character.h"
#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetSystemLibrary.h"


UR1CameraOcclusionComponent::UR1CameraOcclusionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UR1CameraOcclusionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UR1CameraOcclusionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AR1Character* PlayerCharacter = Cast<AR1Character>(GetOwner());
	if (!PlayerCharacter) return;

	UCameraComponent* CameraComp = PlayerCharacter->FindComponentByClass<UCameraComponent>();
	if (!CameraComp) return;

	// 1. 카메라 → 플레이어 몸통 방향으로 Sphere Sweep
	FVector StartLocation = CameraComp->GetComponentLocation();
	FVector EndLocation = PlayerCharacter->GetActorLocation() + FVector(0.f, 0.f, 50.f);

	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PlayerCharacter);
	QueryParams.bReturnFaceIndex = true; // ISM 인스턴스 인덱스(Hit.Item) 획득에 필요

	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(CheckRadius);
	GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_Camera,
		CollisionShape,
		QueryParams
	);

	// 2. 이번 프레임에 히트된 (컴포넌트, 인스턴스) 쌍 수집
	TMap<UPrimitiveComponent*, TSet<int32>> CurrentHits;

	for (const FHitResult& Hit : HitResults)
	{
		UPrimitiveComponent* HitComp = Hit.GetComponent();
		if (!HitComp) continue;

		// ISM이면 Hit.Item = 인스턴스 인덱스, 일반 Mesh이면 INDEX_NONE
		UInstancedStaticMeshComponent* ISMComp = Cast<UInstancedStaticMeshComponent>(HitComp);
		int32 InstanceIdx = ISMComp ? Hit.Item : INDEX_NONE;

		CurrentHits.FindOrAdd(HitComp).Add(InstanceIdx);

		// 처음 히트된 경우 등록
		RegisterOccludedComponent(HitComp, InstanceIdx);

		// 타깃 투명도 설정
		if (FOcclusionComponentData* CompData = OccludedComponentMap.Find(HitComp))
		{
			if (FOcclusionInstanceData* InstData = CompData->Instances.Find(InstanceIdx))
			{
				InstData->TargetOpacity = OccludedOpacity;
			}
		}
	}

	// 3. 투명도 보간(Fading) 및 복구 로직
	for (auto CompIt = OccludedComponentMap.CreateIterator(); CompIt; ++CompIt)
	{
		UPrimitiveComponent* Comp = CompIt.Key();
		FOcclusionComponentData& CompData = CompIt.Value();

		bool bAllRestored = true;

		for (auto& InstPair : CompData.Instances)
		{
			int32 InstIdx = InstPair.Key;
			FOcclusionInstanceData& InstData = InstPair.Value;

			// 이번 프레임 히트에 없으면 복구 준비
			TSet<int32>* HitSet = CurrentHits.Find(Comp);
			if (!HitSet || !HitSet->Contains(InstIdx))
			{
				InstData.TargetOpacity = 1.0f;
			}

			// 부드러운 투명도 전환
			InstData.CurrentOpacity = FMath::FInterpTo(
				InstData.CurrentOpacity, InstData.TargetOpacity, DeltaTime, FadeSpeed);

			// 실제 머티리얼 적용
			if (CompData.bIsISM)
			{
				// ISM/HISM: 인스턴스별 CustomData로 개별 제어
				if (UInstancedStaticMeshComponent* ISMComp = Cast<UInstancedStaticMeshComponent>(Comp))
				{
					ISMComp->SetCustomDataValue(InstIdx, PerInstanceDataIndex, InstData.CurrentOpacity, /*bMarkRenderStateDirty=*/true);
				}
			}
			else
			{
				// 일반 StaticMesh: MID 파라미터 업데이트
				for (UMaterialInstanceDynamic* MID : CompData.MIDs)
				{
					if (MID) MID->SetScalarParameterValue(OpacityParamName, InstData.CurrentOpacity);
				}
			}

			// 완전 복구 여부 체크
			if (!(InstData.TargetOpacity >= 1.0f && FMath::IsNearlyEqual(InstData.CurrentOpacity, 1.0f, 0.01f)))
			{
				bAllRestored = false;
			}
		}

		// 모든 인스턴스가 완전 복구됐으면 맵에서 제거 (최적화)
		if (bAllRestored)
		{
			if (CompData.bIsISM)
			{
				if (UInstancedStaticMeshComponent* ISMComp = Cast<UInstancedStaticMeshComponent>(Comp))
				{
					for (auto& InstPair : CompData.Instances)
					{
						ISMComp->SetCustomDataValue(InstPair.Key, PerInstanceDataIndex, 1.0f, true);
					}
				}
			}
			else
			{
				for (UMaterialInstanceDynamic* MID : CompData.MIDs)
				{
					if (MID) MID->SetScalarParameterValue(OpacityParamName, 1.0f);
				}
			}
			CompIt.RemoveCurrent();
		}
	}
}

void UR1CameraOcclusionComponent::RegisterOccludedComponent(UPrimitiveComponent* Comp, int32 InstanceIndex)
{
	// 이미 등록된 컴포넌트라면 인스턴스 인덱스만 추가
	if (FOcclusionComponentData* Existing = OccludedComponentMap.Find(Comp))
	{
		if (!Existing->Instances.Contains(InstanceIndex))
		{
			Existing->Instances.Add(InstanceIndex, FOcclusionInstanceData{ InstanceIndex, 1.0f, OccludedOpacity });
		}
		return;
	}

	// 신규 등록
	FOcclusionComponentData NewData;
	UInstancedStaticMeshComponent* ISMComp = Cast<UInstancedStaticMeshComponent>(Comp);

	if (ISMComp)
	{
		// ISM/HISM 경로: PerInstanceCustomData 방식
		NewData.bIsISM = true;

		if (ISMComp->NumCustomDataFloats < (PerInstanceDataIndex + 1))
		{
			UE_LOG(LogTemp, Warning,
				TEXT("R1CameraOcclusion: ISM '%s'의 NumCustomDataFloats(%d)가 부족합니다."
					 " 에디터에서 NumCustomDataFloats를 최소 %d 이상으로 설정해주세요."),
				*Comp->GetName(), ISMComp->NumCustomDataFloats, PerInstanceDataIndex + 1);
		}
	}
	else
	{
		// 일반 StaticMesh 경로: 항상 새 MID 생성 (공유 MID 버그 수정)
		NewData.bIsISM = false;
		if (UMeshComponent* MeshComp = Cast<UMeshComponent>(Comp))
		{
			for (int32 i = 0; i < MeshComp->GetNumMaterials(); ++i)
			{
				UMaterialInterface* Mat = MeshComp->GetMaterial(i);
				if (Mat)
				{
					// ★ 기존 MID 여부와 관계없이 항상 새 MID를 생성해 공유 문제 방지
					UMaterialInstanceDynamic* NewMID = MeshComp->CreateDynamicMaterialInstance(i, Mat);
					if (NewMID) NewData.MIDs.Add(NewMID);
				}
			}
		}
	}

	NewData.Instances.Add(InstanceIndex, FOcclusionInstanceData{ InstanceIndex, 1.0f, OccludedOpacity });
	OccludedComponentMap.Add(Comp, NewData);
}
