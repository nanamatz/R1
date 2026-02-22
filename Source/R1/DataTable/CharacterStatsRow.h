

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
	float MaxHealth = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float HealthRegeneration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseDefence = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackRange = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackRadius = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackSpeed = 0.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MoveSpeed = 0.0f;


	//Player Only
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxMana = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ManaRegeneration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxExp = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Level = 0.0f;

	//Monster Only
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Xp = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AggroRange = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackAngle = 0.0f;
};
