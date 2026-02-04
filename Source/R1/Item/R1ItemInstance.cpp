


#include "Item/R1ItemInstance.h"

UR1ItemInstance::UR1ItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}


void UR1ItemInstance::Init(int32 InItemID)
{
	if (InItemID <= 0)
	{
		return;
	}

	ItemID = InItemID;
	ItemRarity = EItemRarity::Common;
}