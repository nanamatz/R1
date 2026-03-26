

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/R1ShopNPCData.h"
#include "R1NPCPoolData.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1NPCPoolData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	// 이 풀에서 등장할 수 있는 NPC 데이터 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Pool")
	TArray<TObjectPtr<UR1ShopNPCData>> AvailableNPCs;
};
