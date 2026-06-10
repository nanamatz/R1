

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "R1GameplayAbility_JumpAttack.generated.h"

/**
 * 
 */
class AR1Monster;
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
private:
	UFUNCTION()
	void OnDashFinished();

	UFUNCTION()
	void OnDashInterrupted();

	// 루트모션(대시) 종료 시 호출. 실제 착지 시점에 맞춰 착지 처리를 수행한다.
	// 델리게이트 시그니처(bReachedDestination, bTimedOut, FinalTargetLocation)에 맞춰야 한다.
	UFUNCTION()
	void OnDashRootMotionFinished(bool bReachedDestination, bool bTimedOut, FVector FinalTargetLocation);

	UFUNCTION()
	void OnJumpAttackEventReceived(FGameplayEventData Payload);

protected:
	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	TSubclassOf<class UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	float DashDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack")
	FGameplayTag AttackEventTag;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack|Audio")
	FGameplayTag AudioTag;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack|Audio")
	FGameplayTag GameplayCueTag;

	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	TObjectPtr<class UCurveVector> JumpHeightCurve; // 포물선을 그리기 위한 커브 (옵션)

private:
	UPROPERTY()
	TObjectPtr<AR1Monster> CachedTarget;

	UPROPERTY()
	FVector CachedTargetLocation;
};
