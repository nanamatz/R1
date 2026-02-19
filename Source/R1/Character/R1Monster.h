

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

	class UR1AttributeSet* GetR1AttributeSet() const { return AttributeSet; }

public:
	void ActivateAbility(FGameplayTag AbilityTag);
public:
	virtual void OnDead(const TObjectPtr<class AR1Character> Attacker) override;

public:
	// 스포너가 태어날 때 호출해 줄 주입 함수
	void InitializeWithManager(class ADungeonManager* InManager);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TObjectPtr<class UAnimMontage> DeathAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<class UGameplayEffect> XpEffect;

	float AggroRange;


};
