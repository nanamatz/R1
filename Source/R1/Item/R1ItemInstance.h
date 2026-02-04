

#pragma once

#include "CoreMinimal.h"
#include "R1Define.h"
#include "UObject/NoExportTypes.h"
#include "R1ItemInstance.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class R1_API UR1ItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UR1ItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void Init(int32 InItemID);
	
public:
	UPROPERTY()
	int32 ItemID = 0;

	UPROPERTY()
	EItemRarity ItemRarity = EItemRarity::Junk;
};
