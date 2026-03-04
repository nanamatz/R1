

#pragma once



UENUM(BlueprintType)
enum class ER1SkillSlot : uint8
{
	None,
	Q,
	W,
	E,
	R
};

UENUM(BlueprintType)
enum class ER1EquipmentSlot : uint8
{
	None,		// 장비 아님 (포션, 재료 등)
	Weapon,		
	Helmet,		
	Armor,		
	Glove,
	Ring1,		
	Ring2,		
	Boots		
};

UENUM(BlueprintType)
enum class ECreatureState: uint8
{
	Idle,
	Moving,
	Casting,
	Dead,
};

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Junk,
	Poor,
	Common,
	Uncommon,
	Rare,
	Epic,
	Legendary,
	Unique,

	Count	UMETA(Hidden)
};

namespace Item
{
	const FIntPoint UnitInventorySlotSize = FIntPoint(50.f, 50.f);
};
