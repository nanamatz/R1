

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PlayerDataTable.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FR1CharacterData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // 엑셀의 컬럼이 된다고 생각하시면 됩니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxMana = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackPower = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Defense = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRadius = 0.0f;
};

