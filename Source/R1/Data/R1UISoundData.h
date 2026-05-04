
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "R1UISoundData.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1UISoundData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(FName("UISoundData"), GetFName());
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TObjectPtr<USoundBase> ShopPurchase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<USoundBase> InventoryFullError;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<USoundBase> PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	TObjectPtr<USoundBase> ActionError;
};
