#pragma once

#include "CoreMinimal.h"
#include "Character/R1Monster.h"
#include "R1Boss.generated.h"

/**
 * 
 */
UCLASS()
class R1_API AR1Boss : public AR1Monster
{
	GENERATED_BODY()
public:
    AR1Boss();

protected:
    virtual void BeginPlay() override;
    virtual void OnDead(const TObjectPtr<class AR1Character> Attacker) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Loot")
    TObjectPtr<class UR1ItemPoolData> BossLootPool;

    UPROPERTY(EditAnywhere, Category = "Loot")
    float BossDropChance = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Loot")
    TSubclassOf<class AR1ItemActor> ItemActorClass;
};
