

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "R1Define.h"
#include "R1ItemDataRow.generated.h"


class UR1GameplayAbility;
class UGameplayEffect;

// 💡 아이템의 대분류
UENUM(BlueprintType)
enum class ER1ItemType : uint8
{
	None		UMETA(DisplayName = "없음"),
	Equipment	UMETA(DisplayName = "장비"),
	Consumable	UMETA(DisplayName = "소모품"),
	Material	UMETA(DisplayName = "재료")
};

USTRUCT(BlueprintType)
struct R1_API FR1ItemDataRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	// --- [공통 기본 정보] ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Basic")
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Basic")
	UTexture2D* ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Basic")
	FIntPoint ItemSize = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Basic")
	ER1ItemType ItemType = ER1ItemType::Equipment;

	// --- [장비 전용 정보] ---
	// 💡 마법의 코드: ItemType이 Equipment일 때만 에디터에 보입니다!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Equipment", meta = (EditCondition = "ItemType == ER1ItemType::Equipment", EditConditionHides))
	ER1EquipmentSlot EquipSlot = ER1EquipmentSlot::Weapon;

	// --- [GAS 능력치 및 스킬] ---
	// 장비: 장착 시 영구 적용되는 스탯 (최대 체력 증가 등)
	// 소모품: 우클릭 시 즉시 적용되는 효과 (체력 즉시 회복 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. GAS")
	TSubclassOf<UGameplayEffect> GrantedEffect;

	// 장비: 장착 시 발동 가능한 고유 스킬 (화염 베기 등)
	// 소모품: 우클릭 시 시전되는 스킬 (메테오 스크롤 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. GAS")
	TSubclassOf<UR1GameplayAbility> GrantedAbility;
};
