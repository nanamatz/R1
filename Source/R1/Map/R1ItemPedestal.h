

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "R1ItemPedestal.generated.h"

UCLASS()
class R1_API AR1ItemPedestal : public AActor
{
	GENERATED_BODY()
	
public:
	AR1ItemPedestal();

protected:
	virtual void BeginPlay() override;

public:
	// 🌟 제단의 3D 외형 (돌기둥 모양 메쉬 등을 에디터에서 넣어줍니다)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> PedestalMesh;

	// 🌟 이 제단 전용 보물방 보상 풀 (DA_TreasureLootPool 할당)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<class UR1ItemPoolData> TreasureLootPool;

	// 🌟 스폰할 아이템 액터 클래스 (BP_ItemActor 할당)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TSubclassOf<class AR1ItemActor> ItemActorClass;

};
