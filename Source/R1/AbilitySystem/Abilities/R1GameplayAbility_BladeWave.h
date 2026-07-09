#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "R1Define.h"
#include "R1GameplayAbility_BladeWave.generated.h"

class AR1BladeWaveProjectile;
class UGameplayEffect;

/**
 * 블레이드 웨이브: 스킬 키를 누르고 있는 동안 차지, 떼면 커서 방향으로 관통 검기 발사.
 * - 차지 0.5s(최소)~3s(최대), 최대 차지 상태에서는 키를 뗄 때까지 유지
 * - 데미지·크기가 차지 비율로 스케일
 * - 마나/쿨다운은 발사 시점에만 지불 (조기 취소는 무료)
 * - 차지 중 피격 리액션 차단 (하이퍼아머, 데미지는 그대로 받음)
 */
UCLASS()
class R1_API UR1GameplayAbility_BladeWave : public UR1GameplayAbility
{
	GENERATED_BODY()

public:
	UR1GameplayAbility_BladeWave(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual void OnMontageEnded() override;
	// 발사 노티파이(Event.Montage.BladeWave) 수신 → 투사체 스폰 + 비용/쿨다운 커밋
	virtual void OnAttackEventReceived(FGameplayEventData Payload) override;

	UFUNCTION()
	void OnSkillKeyReleased(FGameplayEventData Payload);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Data")
	FName SkillID = FName("BladeWave");

	UPROPERTY(EditAnywhere, Category = "BladeWave")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere, Category = "BladeWave")
	TSubclassOf<AR1BladeWaveProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "BladeWave")
	float ProjectileSpeed = 1200.0f;

	// [설정] 애니메이션에서 보낼 발사 이벤트 태그 (Event.Montage.BladeWave)
	UPROPERTY(EditDefaultsOnly, Category = "BladeWave")
	FGameplayTag AttackEventTag;

	UPROPERTY(EditAnywhere, Category = "BladeWave|Charge")
	float MinChargeTime = 0.5f;

	UPROPERTY(EditAnywhere, Category = "BladeWave|Charge")
	float MaxChargeTime = 3.0f;

	UPROPERTY(EditAnywhere, Category = "BladeWave|Charge")
	float MinDamageScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "BladeWave|Charge")
	float MaxDamageScale = 2.5f;

	UPROPERTY(EditAnywhere, Category = "BladeWave|Charge")
	float MinSizeScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "BladeWave|Charge")
	float MaxSizeScale = 2.0f;

	// 몽타주의 발사 섹션 이름
	UPROPERTY(EditAnywhere, Category = "Animation")
	FName FireSectionName = FName("Fire");

private:
	float ChargeStartTime = 0.0f;
	float CachedDamageScale = 1.0f;
	float CachedSizeScale = 1.0f;
	FVector CachedFireDirection = FVector::ForwardVector;
	bool bReleased = false;

	// DT_SkillData에서 OnAvatarSet 시점에 캐싱
	float CachedSkillDamage = 0.0f;
	float CachedManaCost = 0.0f;
	float CachedCooldown = 0.0f;
	float CachedRange = 0.0f;
};
