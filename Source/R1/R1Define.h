

#pragma once

UENUM(BlueprintType)
enum class ER1EquipmentSlot : uint8
{
	None,		// 장비 아님 (포션, 재료 등)
	Weapon,		// 무기 (예: 2x3)
	Helmet,		// 투구 (예: 2x2)
	Armor,		// 갑옷 (예: 2x2)
	Ring,		// 반지 (예: 1x1)
	Boots		// 신발 (예: 2x2)
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
