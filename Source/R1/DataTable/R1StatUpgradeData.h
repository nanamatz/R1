#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "R1StatUpgradeData.generated.h"

UENUM(BlueprintType)
enum class ER1StatDisplayType : uint8 { 
    /** Display as a whole number. */
    Integer, 
    /** Display as a decimal value. */
    Float, 
    /** Display as a fraction (e.g., 1/2). */
    Fraction, 
    /** Display as a range (e.g., 10-20). */
    Range 
};

USTRUCT(BlueprintType)
struct FR1StatUpgradeData : public FTableRowBase {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Upgrade", meta = (Categories = "Stat.Run.Upgrade")) FGameplayTag StatTag;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Upgrade") FText StatName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Upgrade") FGameplayAttribute Attribute;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Upgrade") float IncreaseAmount;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Upgrade") ER1StatDisplayType DisplayType;
};
