

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CharacterStatsRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FR1CharacterStatsRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxMana = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseDamage = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseDefence = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackRange = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackRadius = 0.0f;
};
