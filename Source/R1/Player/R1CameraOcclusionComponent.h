

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "R1CameraOcclusionComponent.generated.h"

USTRUCT()
struct FOcclusionData
{
	GENERATED_BODY()

	// 교체된 다이내믹 머티리얼 인스턴스(MID) 배열
	UPROPERTY()
	TArray<class UMaterialInstanceDynamic*> MIDs;

	float CurrentOpacity = 1.0f;
	float TargetOpacity = 1.0f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class R1_API UR1CameraOcclusionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UR1CameraOcclusionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// 🌟 설정 변수들
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	float OccludedOpacity = 0.2f; // 벽이 가려졌을 때의 투명도

	UPROPERTY(EditAnywhere, Category = "Occlusion")
	float FadeSpeed = 5.0f; // 투명해지고 다시 원래대로 돌아오는 속도

	UPROPERTY(EditAnywhere, Category = "Occlusion")
	float CheckRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Occlusion")
	FName OpacityParamName = TEXT("Opacity"); // 머티리얼의 파라미터 이름

	// 현재 투명화 처리 중인 액터들을 관리하는 맵
	UPROPERTY()
	TMap<AActor*, FOcclusionData> OccludedActorMap;

private:
	// 다이내믹 머티리얼 인스턴스를 생성하고 저장하는 함수
	void InitializeActorMIDs(AActor* TargetActor);
		
};
