

#pragma once

UENUM(BlueprintType)
enum class ER1ItemType : uint8
{
	None		UMETA(DisplayName = "없음"),
	Equipment	UMETA(DisplayName = "장비"),
	Consumable	UMETA(DisplayName = "소모품"),
	Material	UMETA(DisplayName = "재료")
};

UENUM(BlueprintType)
enum class ER1FloorLevel : uint8
{
	Laboratory,
	Factory,
	SecurityArea,
	RestrictedArea,
	Robby,
	Basement,
	Ground,
	EmptyHall,
	Rooftop
};

UENUM(BlueprintType)
enum class ER1MinimapRoomState : uint8
{
	Hidden,
	Discovered,
	Current,
	Visited
};

UENUM(BlueprintType)
enum class EMenuState : uint8
{
	Title,
	MainMenu,
	Options
};

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
	Common,
	Uncommon,
	Rare,
	Epic,
	Legendary,

	Count	UMETA(Hidden)
};

namespace Item
{
	const FIntPoint UnitInventorySlotSize = FIntPoint(50.f, 50.f);
};
