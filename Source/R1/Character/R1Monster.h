

#pragma once

#include "CoreMinimal.h"
#include "Character/R1Character.h"
#include "R1Monster.generated.h"

/**
 * 
 */
UCLASS()
class R1_API AR1Monster : public AR1Character
{
	GENERATED_BODY()

public:
	AR1Monster();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void InitAbilitySystem() override;

	//void DefaultAttack();
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TObjectPtr<class UWidgetComponent> HpBarComponent;

	UFUNCTION()
	void RefreshHpBar(float Ratio);

public:
	void ActivateAbility(FGameplayTag AbilityTag);
public:
	virtual void OnDead(const TObjectPtr<class AR1Character> Attacker) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<class UAnimMontage> DeathAnimMontage;

	float AggroRange = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GE")
	TSubclassOf<class UGameplayEffect> XpEffect;
};
