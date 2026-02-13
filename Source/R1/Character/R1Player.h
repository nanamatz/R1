

#pragma once

#include "CoreMinimal.h"
#include "Character/R1Character.h"
#include "R1Player.generated.h"

/**
 * 
 */
UCLASS()
class R1_API AR1Player : public AR1Character
{
	GENERATED_BODY()

public:
	AR1Player( );
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UCameraComponent> Camera;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void InitAbilitySystem() override;

//public:
//	virtual void SetCreatureState(ECreatureState InState) override;
//	virtual ECreatureState GetCreatureState() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void HandleGameplayEvent(FGameplayTag EventTag) override;

	virtual void OnDead(const TObjectPtr<AR1Character> Attacker) override;

public:
	class UR1AttributeSet* GetR1AttributeSet() const { return AttributeSet; }

public:
	void ActivateAbility(FGameplayTag AbilityTag);

public:
	float AttackRange;

};
