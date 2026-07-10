

#pragma once
#include "R1Define.generated.h"



UENUM(BlueprintType)
enum class ER1TelegraphShape : uint8
{
	Circle,
	Cone,
	Rectangle
};

UENUM(BlueprintType)
enum class ER1CharacterClass : uint8
{
	Knight,
	OutLaw,
	Fighter,
	Wizard
};

UENUM(BlueprintType)
enum class ER1ItemType : uint8
{
	Equipment	UMETA(DisplayName = "장비"),
	Consumable	UMETA(DisplayName = "소모품"),
	Material	UMETA(DisplayName = "재료"),
	Key
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
	Options,
	MetaUpgrade
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

// 무기 포즈 카테고리 — 개별 무기 종류가 아니라 ABP가 사용할 애니메이션 포즈 묶음.
// (도끼/망치/단검=OneHanded, 검=TwoHanded, 너클=Unarmed 포즈 재사용)
UENUM(BlueprintType)
enum class ER1WeaponType : uint8
{
	Unarmed		UMETA(DisplayName = "맨손"),
	OneHanded	UMETA(DisplayName = "한손 무기"),
	TwoHanded	UMETA(DisplayName = "양손 무기"),
	Pistol		UMETA(DisplayName = "권총"),
	Shuriken	UMETA(DisplayName = "수리검")
};

// 아이템/무기의 속성 원소. 현재는 Fire만 콘텐츠 존재, 나머지는 예약값.
UENUM(BlueprintType)
enum class ER1ElementType : uint8
{
	None		UMETA(DisplayName = "무속성"),
	Fire		UMETA(DisplayName = "화염"),
	Ice			UMETA(DisplayName = "냉기"),
	Lightning	UMETA(DisplayName = "번개"),
	Poison		UMETA(DisplayName = "독"),
	Dark		UMETA(DisplayName = "암흑"),
	Earth		UMETA(DisplayName = "대지"),
	Water		UMETA(DisplayName = "물"),
	Wind		UMETA(DisplayName = "물"),
	Wave		UMETA(DisplayName = "파동"),
	Matrix		UMETA(DisplayName = "매트릭스")
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

UENUM(BlueprintType)
enum class ER1DamageType : uint8
{
	Normal,
	Critical,
	Heal
};

USTRUCT(BlueprintType)
struct FR1DamageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DamageAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ER1DamageType DamageType = ER1DamageType::Normal;
};
