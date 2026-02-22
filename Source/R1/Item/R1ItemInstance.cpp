


#include "Item/R1ItemInstance.h"

UR1ItemInstance::UR1ItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UR1ItemInstance::Init(int32 InItemID, FIntPoint InItemSize, ER1EquipmentSlot InEquipSlot)
{
	if (InItemID <= 0)
	{
		return;
	}

	ItemID = InItemID;
	ItemRarity = EItemRarity::Common;

	// 초기화 시 전달받은 크기와 슬롯 정보를 저장합니다.
	ItemSize = InItemSize;
	EquipSlot = InEquipSlot;
}