#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility_BossAttackBase.h"
#include "R1GameplayAbility_BossCharge.generated.h"

class UGameplayEffect;
class AR1Character;

/**
 * 보스 돌진 공격. 텔레그래프로 예고한 직선 레인을 그대로 밀고 들어간다.
 * 타겟을 추적하지 않으며(예고한 레인 = 실제 경로), 플레이어를 밀지 않고 통과한다.
 * 지나가는 동안 스쳐간 대상에게 각 1회만 피해를 준다.
 *
 * 윈드업(텔레그래프 + 몽타주 + Event.Montage.Attack 대기)은 베이스가 그대로 처리한다.
 * 이 클래스는 "타겟을 향해 회전"과 "노티파이 시점에 돌진 시작"만 담당한다.
 */
UCLASS()
class R1_API UR1GameplayAbility_BossCharge : public UR1GameplayAbility_BossAttackBase
{
	GENERATED_BODY()

public:
	// 베이스가 FObjectInitializer 생성자를 노출하지 않으므로 기본 생성자로 맞춘다.
	UR1GameplayAbility_BossCharge();

protected:
	// 베이스의 텔레그래프가 돌진 레인과 나란히 깔리도록, Super 호출 전에 타겟 쪽으로 회전한다.
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 몽타주 노티파이(돌진 시작 프레임)에서 루트모션 대시를 시작한다.
	virtual void OnAttackEventReceived(FGameplayEventData Payload) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	// ApplyRootMotionMoveToForce의 OnTimedOut / OnTimedOutAndDestinationReached는
	// 둘 다 파라미터가 없는 델리게이트다. 어느 쪽으로 끝나든 처리는 같다.
	UFUNCTION()
	void OnChargeFinished();

	// 이전 프레임 위치 → 현재 위치 구간을 스윕해 새로 스친 대상에게만 피해를 준다.
	void SweepAndDamage();

	// 돌진 중 폰 충돌을 Overlap으로 낮춘다(통과용). 종료 시 원복.
	void SetPawnCollisionPassThrough(bool bPassThrough);

	void StopChargeSweep();

protected:
	UPROPERTY(EditAnywhere, Category = "BossCharge")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// 돌진 거리(cm). 텔레그래프 Rectangle의 X와 같은 값으로 맞출 것.
	UPROPERTY(EditAnywhere, Category = "BossCharge", meta = (ClampMin = "0.0"))
	float ChargeDistance = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "BossCharge", meta = (ClampMin = "0.05"))
	float ChargeDuration = 0.7f;

	// 스윕 반경. 텔레그래프 Rectangle의 Y(폭)의 절반으로 맞출 것.
	UPROPERTY(EditAnywhere, Category = "BossCharge", meta = (ClampMin = "0.0"))
	float ChargeHalfWidth = 150.0f;

	// 스윕 주기(초). 0.033 = 약 30Hz.
	UPROPERTY(EditAnywhere, Category = "BossCharge", meta = (ClampMin = "0.008"))
	float SweepInterval = 0.033f;

	// 타겟을 읽어올 블랙보드 키 이름. BB_Boss 기준 "TargetActor".
	UPROPERTY(EditAnywhere, Category = "BossCharge")
	FName BBKey_TargetActor = FName("TargetActor");

private:
	// 이번 돌진에서 이미 맞은 대상 (중복 타격 방지)
	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActors;

	FVector LastSweepLocation = FVector::ZeroVector;

	FTimerHandle SweepTimerHandle;

	// 원복용 — 돌진 전 폰 충돌 응답
	TEnumAsByte<ECollisionResponse> CachedPawnResponse = ECR_Block;

	bool bCollisionModified = false;
};
