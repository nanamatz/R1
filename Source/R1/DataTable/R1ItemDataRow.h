

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "R1Define.h"
#include "R1GameplayTags.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	UTexture2D* ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FIntPoint ItemSize = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	ER1ItemType ItemType = ER1ItemType::Equipment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TArray<ER1EquipmentSlot> EquipSlots;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TSubclassOf<UGameplayEffect> EquipStatEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TArray<TSubclassOf<UR1GameplayAbility>> GrantedAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TMap<FGameplayTag, float> StatModifiers;
};
