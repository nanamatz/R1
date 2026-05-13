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

public:
	TArray<TSubclassOf<class UGameplayAbility>> GetDefaultSkillList() { return DefaultSkillAbilities; }
	TArray<TSubclassOf<class UGameplayAbility>> GetAdditionalSkillList() { return AdditionalSkillAbilities; }
protected:
    virtual void BeginPlay() override;
    virtual void OnDead(const TObjectPtr<class AR1Character> Attacker) override;
	virtual void AddCharacterAbility() override;

protected:
	void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> DefaultSkillAbilities;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> AdditionalSkillAbilities;
};
