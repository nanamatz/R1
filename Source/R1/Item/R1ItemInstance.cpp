


#include "Item/R1ItemInstance.h"
#include "Engine/DataTable.h"
#include "UObject/UObjectGlobals.h"

UR1ItemInstance::UR1ItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UR1ItemInstance::Init(UR1ItemAssetData* InItemData, EItemRarity InRarity)
{
	ItemData = InItemData;
	ItemRarity = InRarity;
}

