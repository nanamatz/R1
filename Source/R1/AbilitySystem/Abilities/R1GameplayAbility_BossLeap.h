#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility_BossAttackBase.h"
#include "R1GameplayAbility_BossLeap.generated.h"

class UGameplayEffect;
class UCurveVector;

/**
 * 보스 전용 돌진 공격. 블랙보드 TargetActor를 향해 루트모션으로 도약하고,
 * 착지 지점에 원형 범위 피해를 준다.
 * JumpAttack은 AR1Player + PlayerController(하이라이트 타겟)에 묶여 있어 AI가 쓸 수 없으므로 별도 구현.
 */
UCLASS()
class R1_API UR1GameplayAbility_BossLeap : public UR1GameplayAbility_BossAttackBase
{
	GENERATED_BODY()

public:
	// 베이스(BossAttackBase)가 FObjectInitializer 생성자를 노출하지 않으므로 기본 생성자로 맞춘다.
	UR1GameplayAbility_BossLeap();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	// 루트모션 종료 시 착지 판정.
	// ApplyRootMotionMoveToForce의 OnTimedOut / OnTimedOutAndDestinationReached는
	// 둘 다 파라미터가 없다. 어느 쪽으로 끝나든 처리는 같다.
	UFUNCTION()
	void OnLeapFinished();

	// 착지 지점 구체 판정 + 피해 적용
	void ApplyLandingDamage(const FVector& LandingLocation);

protected:
	UPROPERTY(EditAnywhere, Category = "BossLeap")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere, Category = "BossLeap")
	float DashDuration = 0.6f;

	// 포물선용 커브 (선택).
	UPROPERTY(EditAnywhere, Category = "BossLeap")
	TObjectPtr<UCurveVector> JumpHeightCurve;

	// 타겟을 읽어올 블랙보드 키 이름. BB_Boss 기준 "TargetActor".
	UPROPERTY(EditAnywhere, Category = "BossLeap")
	FName BBKey_TargetActor = FName("TargetActor");

private:
	UPROPERTY()
	TObjectPtr<AActor> CachedTarget;

	// 텔레그래프를 깐 좌표. 도약은 타겟을 추적하지 않고 이 지점에 착지한다 —
	// 추적하면 예고 원과 실제 착지점이 어긋나 회피가 불가능해진다.
	FVector CachedLandingLocation = FVector::ZeroVector;
};
