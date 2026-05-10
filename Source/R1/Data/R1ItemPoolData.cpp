


#include "Data/R1ItemPoolData.h"
#include "Data/R1ItemAssetData.h"

UR1ItemAssetData* UR1ItemPoolData::GetRandomItemFromPool(const UR1ItemPoolData* Pool)
{
	if (!Pool || Pool->DropItems.IsEmpty())
	{
		return nullptr;
	}

	float TotalWeight = 0.0f;
	for (UR1ItemAssetData* ItemData : Pool->DropItems)
	{
		if (ItemData)
		{
			TotalWeight += ItemData->GetDropWeight();
		}
	}

	if (TotalWeight <= 0.0f)
	{
		return nullptr;
	}

	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float AccumulatedWeight = 0.0f;

	for (UR1ItemAssetData* ItemData : Pool->DropItems)
	{
		if (!ItemData) continue;

		AccumulatedWeight += ItemData->GetDropWeight();
		if (RandomValue <= AccumulatedWeight)
		{
			return ItemData;
		}
	}

	return nullptr;
}

