

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "R1GameplayAbility.generated.h"

UENUM(BlueprintType)
enum class ER1SkillType : uint8
{
	Active,
	Passive
};

UCLASS()
class R1_API UR1GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UR1GameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	ER1SkillType GetSkillType() const { return SkillType; }

protected:
	UPROPERTY(EditAnywhere, Category = "Skill")
	ER1SkillType SkillType = ER1SkillType::Passive;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<class UAnimMontage> MontageToPlay;

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<class USoundBase> SoundToPlay;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tooltip", meta = (MultiLine = true))
	FText AbilityDescription;

	UAnimMontage* GetMontageToPlay() const { return MontageToPlay; }
	USoundBase* GetSoundToPlay() const { return SoundToPlay; }
};
