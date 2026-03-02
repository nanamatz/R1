

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "R1GameplayAbility_JumpAttack.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1GameplayAbility_JumpAttack : public UR1GameplayAbility
{
	GENERATED_BODY()
public:
	UR1GameplayAbility_JumpAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
protected:
	// 블루프린트에서 세팅할 데이터 테이블과 행 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Data")
	FName SkillID = FName("JumpAttack");

private:
	float CachedSkillDamage = 0.0f;
	float CachedSkillRange = 0.0f;
	float CachedManaCost = 0.0f;
public:
	UFUNCTION()
	void OnDashFinished(bool bDestinationReached, bool bTimedOut, FVector FinalTargetLocation);

	UFUNCTION()
	void OnDashInterrupted();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack")
	TObjectPtr<class UAnimMontage> JumpMontage;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack")
	TSubclassOf<class UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack")
	float DashDuration = 1.23f; // 체공 시간 (짧을수록 빠르게 날아감)

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack")
	TObjectPtr<class UCurveVector> JumpHeightCurve; // 포물선을 그리기 위한 커브 (옵션)

private:
	UPROPERTY()
	TObjectPtr<class AR1Monster> CachedTarget;
};
