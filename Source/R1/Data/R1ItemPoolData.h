

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/R1ItemAssetData.h"
#include "R1ItemPoolData.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1ItemPoolData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TArray<TObjectPtr<UR1ItemAssetData>> DropItems;

	UFUNCTION(BlueprintCallable, Category = "Loot")
	static class UR1ItemAssetData* GetRandomItemFromPool(const UR1ItemPoolData* Pool);
};
