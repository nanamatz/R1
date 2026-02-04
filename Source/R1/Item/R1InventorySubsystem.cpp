


#include "Item/R1InventorySubsystem.h"
#include "Item/R1ItemInstance.h"

void UR1InventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UR1InventorySubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UR1InventorySubsystem::AddDefaultItem()
{
	TObjectPtr<UR1ItemInstance> Item = NewObject<UR1ItemInstance>();
	Item->Init(1);

	Items.Add(Item);
}
