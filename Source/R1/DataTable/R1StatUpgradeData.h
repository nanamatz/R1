#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "R1StatUpgradeData.generated.h"

UENUM(BlueprintType)
enum class ER1StatDisplayType : uint8 { Integer, Float, Fraction, Range };

USTRUCT(BlueprintType)
struct FR1StatUpgradeData : public FTableRowBase {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag StatTag;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText StatName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayAttribute Attribute;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float IncreaseAmount;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) ER1StatDisplayType DisplayType;
};
