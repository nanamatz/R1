
#include "Player/R1CameraOcclusionComponent.h"
#include "R1LogChannels.h"
#include "Character/R1Character.h"
#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Level.h"


UR1CameraOcclusionComponent::UR1CameraOcclusionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UR1CameraOcclusionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 이미 로드된 모든 액터의 ISM 초기화
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		InitializeActorISMs(*It);
	}

	// 스트리밍으로 나중에 로드되는 서브레벨(던전 룸 등)의 ISM도 초기화되도록 델리게이트 등록.
	// BeginPlay는 플레이어 스폰 시 1회만 실행되므로, 이후 스트림인되는 ISM은 이 훅에서 처리한다.
	LevelAddedHandle = FWorldDelegates::LevelAddedToWorld.AddUObject(
		this, &UR1CameraOcclusionComponent::OnLevelAddedToWorld);
}

void UR1CameraOcclusionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (LevelAddedHandle.IsValid())
	{
		FWorldDelegates::LevelAddedToWorld.Remove(LevelAddedHandle);
		LevelAddedHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void UR1CameraOcclusionComponent::OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld)
{
	// 우리 월드에서 추가된 레벨만 처리 (PIE 등 다른 월드의 브로드캐스트 제외)
	if (!InLevel || InWorld != GetWorld()) return;

	for (AActor* Actor : InLevel->Actors)
	{
		InitializeActorISMs(Actor);
	}
}

void UR1CameraOcclusionComponent::InitializeActorISMs(AActor* Actor)
{
	if (!Actor) return;

	TArray<UInstancedStaticMeshComponent*> ISMComps;
	Actor->GetComponents<UInstancedStaticMeshComponent>(ISMComps);

	for (UInstancedStaticMeshComponent* ISMComp : ISMComps)
	{
		InitializeISMComponent(ISMComp);
	}
}

void UR1CameraOcclusionComponent::InitializeISMComponent(UInstancedStaticMeshComponent* ISMComp)
{
	if (!ISMComp) return;

	const int32 NumInstances = ISMComp->GetInstanceCount();
	if (NumInstances == 0) return;

	const int32 RequiredSlots = PerInstanceDataIndex + 1;
	bool bDirty = false;

	// ── Step 1: NumCustomDataFloats 슬롯 수 확보 (배열 재구성 필요) ──────────
	if (ISMComp->NumCustomDataFloats < RequiredSlots)
	{
		const int32 OldSlots = ISMComp->NumCustomDataFloats;
		ISMComp->NumCustomDataFloats = RequiredSlots;

		TArray<float> NewData;
		NewData.SetNumZeroed(NumInstances * RequiredSlots);

		for (int32 i = 0; i < NumInstances; ++i)
		{
			// 기존 슬롯 데이터 유지
			for (int32 j = 0; j < OldSlots; ++j)
			{
				const int32 OldIdx = i * OldSlots + j;
				if (ISMComp->PerInstanceSMCustomData.IsValidIndex(OldIdx))
					NewData[i * RequiredSlots + j] = ISMComp->PerInstanceSMCustomData[OldIdx];
			}
			// 신규 슬롯은 아래 Step 3에서 1.0으로 덮어씀
		}

		ISMComp->PerInstanceSMCustomData = MoveTemp(NewData);
		bDirty = true;
	}

	// ── Step 2: 배열 크기 보정 (NumCustomDataFloats 충분해도 배열이 비어있을 수 있음) ──
	const int32 ExpectedSize = NumInstances * ISMComp->NumCustomDataFloats;
	if (ISMComp->PerInstanceSMCustomData.Num() < ExpectedSize)
	{
		ISMComp->PerInstanceSMCustomData.SetNumZeroed(ExpectedSize);
		bDirty = true;
	}

	// ── Step 3: PerInstanceDataIndex 슬롯을 1.0(불투명)으로 강제 기록 ──────────
	// NumCustomDataFloats가 이미 충분했더라도(Step 1 스킵) 데이터는 0일 수 있으므로
	// 항상 덮어써야 한다. 이것이 버그1(continue)의 수정 핵심.
	for (int32 i = 0; i < NumInstances; ++i)
	{
		const int32 DataIdx = i * ISMComp->NumCustomDataFloats + PerInstanceDataIndex;
		if (ISMComp->PerInstanceSMCustomData.IsValidIndex(DataIdx))
		{
			ISMComp->PerInstanceSMCustomData[DataIdx] = 1.0f;
			bDirty = true;
		}
	}

	if (bDirty)
	{
		ISMComp->MarkRenderStateDirty();
	}
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

		// 개별 인스턴스 중 복구 완료된 것들을 모아 나중에 제거
		TArray<int32> RestoredInstanceIndices;

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

			// 이 인스턴스가 완전히 복구됐는지 확인
			const bool bInstRestored = InstData.TargetOpacity >= 1.0f
				&& FMath::IsNearlyEqual(InstData.CurrentOpacity, 1.0f, 0.01f);

			if (bInstRestored)
			{
				// 1.0으로 확정하고 제거 목록에 추가
				if (CompData.bIsISM)
				{
					if (UInstancedStaticMeshComponent* ISMComp = Cast<UInstancedStaticMeshComponent>(Comp))
					{
						ISMComp->SetCustomDataValue(InstIdx, PerInstanceDataIndex, 1.0f, true);
					}
				}
				else
				{
					for (UMaterialInstanceDynamic* MID : CompData.MIDs)
					{
						if (MID) MID->SetScalarParameterValue(OpacityParamName, 1.0f);
					}
				}
				RestoredInstanceIndices.Add(InstIdx);
			}
			else
			{
				bAllRestored = false;
			}
		}

		// 복구 완료된 인스턴스를 맵에서 개별 제거
		// (컴포넌트 전체 제거 전에 인스턴스 단위로 정리해 불필요한 처리 제거)
		for (int32 RestoredIdx : RestoredInstanceIndices)
		{
			CompData.Instances.Remove(RestoredIdx);
		}

		// 모든 인스턴스가 제거됐으면 컴포넌트 전체를 맵에서 제거
		if (CompData.Instances.IsEmpty())
		{
			CompIt.RemoveCurrent();
		}
	}
}

void UR1CameraOcclusionComponent::RegisterOccludedComponent(UPrimitiveComponent* Comp, int32 InstanceIndex)
{
	// 이미 등록된 컴포넌트라면 인스턴스 인덱스만 추가 (없는 경우에만)
	if (FOcclusionComponentData* Existing = OccludedComponentMap.Find(Comp))
	{
		if (!Existing->Instances.Contains(InstanceIndex))
		{
			// ★ 버그2 수정: 실제 PerInstanceSMCustomData 값을 읽어 CurrentOpacity로 사용.
			//   1.0f 하드코딩 시 실제값(0.0)과 불일치 → 첫 프레임 pop(순간 불투명) 발생.
			const float ActualOpacity = ReadActualInstanceOpacity(Comp, InstanceIndex);
			Existing->Instances.Add(InstanceIndex, FOcclusionInstanceData{ InstanceIndex, ActualOpacity, OccludedOpacity });
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
			UE_LOG(LogR1, Warning,
				TEXT("R1CameraOcclusion: ISM '%s'의 NumCustomDataFloats(%d)가 부족합니다."
					 " PerInstanceDataIndex=%d — BeginPlay 초기화가 올바르게 실행됐는지 확인하세요."),
				*Comp->GetName(), ISMComp->NumCustomDataFloats, PerInstanceDataIndex);
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
					// 기존 MID 여부와 관계없이 항상 새 MID를 생성해 공유 문제 방지
					UMaterialInstanceDynamic* NewMID = MeshComp->CreateDynamicMaterialInstance(i, Mat);
					if (NewMID) NewData.MIDs.Add(NewMID);
				}
			}
		}
	}

	// ★ 버그2 수정: 실제 현재 opacity 읽기 (ISM의 경우)
	const float ActualOpacity = ReadActualInstanceOpacity(Comp, InstanceIndex);
	NewData.Instances.Add(InstanceIndex, FOcclusionInstanceData{ InstanceIndex, ActualOpacity, OccludedOpacity });
	OccludedComponentMap.Add(Comp, NewData);
}

float UR1CameraOcclusionComponent::ReadActualInstanceOpacity(UPrimitiveComponent* Comp, int32 InstanceIndex) const
{
	// ISM인 경우 실제 PerInstanceSMCustomData 값을 반환.
	// 값을 모르는 경우(배열 범위 초과, 비ISM) 기본값 1.0f 반환.
	if (UInstancedStaticMeshComponent* ISMComp = Cast<UInstancedStaticMeshComponent>(Comp))
	{
		if (ISMComp->NumCustomDataFloats > PerInstanceDataIndex)
		{
			const int32 DataIdx = InstanceIndex * ISMComp->NumCustomDataFloats + PerInstanceDataIndex;
			if (ISMComp->PerInstanceSMCustomData.IsValidIndex(DataIdx))
			{
				return ISMComp->PerInstanceSMCustomData[DataIdx];
			}
		}
	}
	// 일반 StaticMesh는 새 MID를 만들므로 부모 머티리얼의 기본값(=1.0) 가정
	return 1.0f;
}
