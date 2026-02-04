


#include "R1GameMode.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"
#include "Item/R1InventorySubsystem.h"

void AR1GameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UR1InventorySubsystem* Inventory = Cast<UR1InventorySubsystem>(USubsystemBlueprintLibrary::GetWorldSubsystem(this, UR1InventorySubsystem::StaticClass()));

	if (Inventory)
	{
		Inventory->AddDefaultItem();
	}
}
