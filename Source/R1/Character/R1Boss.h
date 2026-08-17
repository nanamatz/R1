#pragma once

#include "CoreMinimal.h"
#include "Character/R1Monster.h"
#include "R1Boss.generated.h"

/**
 * 보스 페이즈 하나. 체력 비율이 임계값 이하로 떨어지면 진입한다.
 */
USTRUCT(BlueprintType)
struct FBossPhase
{
	GENERATED_BODY()

	// Health/MaxHealth가 이 값 이하로 떨어지면 이 페이즈에 진입한다. 내림차순으로 작성할 것.
	UPROPERTY(EditAnywhere, Category = "Phase", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRatioThreshold = 0.5f;

	// 진입 시 근접(사거리 내) 스킬 목록을 교체한다. 비어 있으면 기존 목록 유지.
	UPROPERTY(EditAnywhere, Category = "Phase")
	TArray<TSubclassOf<class UGameplayAbility>> DefaultSkills;

	// 진입 시 원거리(사거리 밖) 스킬 목록을 교체한다. 비어 있으면 기존 목록c 유지.
	UPROPERTY(EditAnywhere, Category = "Phase")
	TArray<TSubclassOf<class UGameplayAbility>> AdditionalSkills;

	// 진입 시 자신에게 적용할 Infinite GE (공속/이속 배율). 제거하지 않으므로 페이즈마다 누적된다.
	UPROPERTY(EditAnywhere, Category = "Phase")
	TSubclassOf<class UGameplayEffect> EnrageEffect;

	// 진입 연출용 몽타주 (선택).
	UPROPERTY(EditAnywhere, Category = "Phase")
	TObjectPtr<class UAnimMontage> TransitionMontage;
};

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
	TArray<TSubclassOf<class UGameplayAbility>> GetDefaultSkillList() { return ActiveDefaultSkills; }
	TArray<TSubclassOf<class UGameplayAbility>> GetAdditionalSkillList() { return ActiveAdditionalSkills; }

	// 페이즈 전환 연출 재생 중인지. BT가 이 동안 새 스킬을 고르지 않게 한다.
	bool IsInPhaseTransition() const { return bIsInPhaseTransition; }

	// 다음 미진입 페이즈의 임계 체력. 전환 연출 중에는 현재 페이즈의 임계값으로 고정된다.
	virtual float GetHealthFloor() const override;

	// 페이즈 진입 시점에 BP로 알린다 (외형 변경, 오라 VFX, 사운드 등 연출용).
	// 스킬 목록 교체 / 격노 GE 적용 / 전환 몽타주 재생이 모두 끝난 뒤 호출된다.
	// 주의: 외형에 SetOverlayMaterial을 쓰지 말 것 — AR1Monster::ResetHitFlash가
	// 피격 0.15초 뒤 오버레이를 nullptr로 지운다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Phases")
	void OnPhaseEntered(int32 PhaseIndex);

protected:
    virtual void BeginPlay() override;
    virtual void OnDead(const TObjectPtr<class AR1Character> Attacker) override;
	virtual void AddCharacterAbility() override;
	virtual void OnHealthChanged(float Ratio, bool bIsDamage) override;

	// Ratio가 다음 페이즈 임계값 이하이면 진입한다. 한 번에 여러 임계값을 넘으면 순서대로 모두 진입한다.
	void AdvancePhasesForRatio(float Ratio);

	// 페이즈 하나에 실제로 진입 (스킬 부여, 목록 교체, 격노 GE, 몽타주, 블랙보드).
	void EnterPhase(int32 PhaseIndex);

	// 전환 연출 시작 — 진행 중인 어빌리티 취소, 이동 정지, 하이퍼아머 태그 부착 후 몽타주 재생.
	// 몽타주가 없거나 재생에 실패하면 전환 상태로 들어가지 않고 false를 반환한다.
	bool BeginPhaseTransition(class UAnimMontage* TransitionMontage);

	UFUNCTION()
	void OnPhaseTransitionMontageEnded(class UAnimMontage* Montage, bool bInterrupted);

	// 전환 종료 처리 (태그 제거, 이동 복구). 몽타주 종료와 사망 양쪽에서 호출된다.
	void EndPhaseTransition();

protected:
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> DefaultSkillAbilities;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> AdditionalSkillAbilities;

	// 내림차순(예: 0.55, 0.20)으로 작성. BeginPlay에서 검증한다.
	UPROPERTY(EditAnywhere, Category = "Phases")
	TArray<FBossPhase> Phases;

	// 현재 페이즈 인덱스. INDEX_NONE = 페이즈 0 진입 전(초기 상태).
	int32 CurrentPhaseIndex = INDEX_NONE;

	// 전환 연출 재생 중 여부. 이 동안 체력 하한이 현재 페이즈 임계값으로 고정된다.
	bool bIsInPhaseTransition = false;

	// 전환 중 이동을 막기 위해 MaxWalkSpeed를 0으로 만들기 전의 값.
	float CachedMaxWalkSpeed = 0.0f;

	// 런타임 스킬 목록. BeginPlay에서 Default/AdditionalSkillAbilities로 초기화된다.
	UPROPERTY(Transient)
	TArray<TSubclassOf<class UGameplayAbility>> ActiveDefaultSkills;

	UPROPERTY(Transient)
	TArray<TSubclassOf<class UGameplayAbility>> ActiveAdditionalSkills;

	// 이미 ASC에 부여한 어빌리티 클래스. 페이즈 진입 시 중복 부여를 막는다.
	UPROPERTY(Transient)
	TSet<TSubclassOf<class UGameplayAbility>> GrantedAbilityClasses;
};
