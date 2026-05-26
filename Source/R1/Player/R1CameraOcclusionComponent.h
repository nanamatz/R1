
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "R1CameraOcclusionComponent.generated.h"

/** 인스턴스 단위 투명도 상태 */
USTRUCT()
struct FOcclusionInstanceData
{
	GENERATED_BODY()

	int32 InstanceIndex = INDEX_NONE;
	float CurrentOpacity = 1.0f;
	float TargetOpacity = 1.0f;
};

/** 컴포넌트 단위 오클루전 데이터 (ISM/HISM과 일반 StaticMesh 모두 지원) */
USTRUCT()
struct FOcclusionComponentData
{
	GENERATED_BODY()

	// 인스턴스 인덱스별 상태 (일반 StaticMesh는 INDEX_NONE 하나만 들어감)
	TMap<int32, FOcclusionInstanceData> Instances;

	// 일반 StaticMesh 전용 MID (ISM이 아닌 경우에만 사용)
	UPROPERTY()
	TArray<class UMaterialInstanceDynamic*> MIDs;

	bool bIsISM = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class R1_API UR1CameraOcclusionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UR1CameraOcclusionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// 🌟 설정 변수들
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	float OccludedOpacity = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Occlusion")
	float FadeSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Occlusion")
	float CheckRadius = 300.0f;

	/** 일반 StaticMesh 머티리얼의 Opacity 파라미터 이름 */
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	FName OpacityParamName = TEXT("Opacity");

	/** ISM/HISM PerInstanceCustomData의 DataIndex (머티리얼 설정값과 일치해야 함) */
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	int32 PerInstanceDataIndex = 0;

	/** 컴포넌트 + 인스턴스 단위로 관리하는 오클루전 맵 */
	UPROPERTY()
	TMap<class UPrimitiveComponent*, FOcclusionComponentData> OccludedComponentMap;

private:
	/** 컴포넌트/인스턴스를 맵에 등록 (최초 히트 시 호출) */
	void RegisterOccludedComponent(class UPrimitiveComponent* Comp, int32 InstanceIndex);
};
