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

	// 진입 시 원거리(사거리 밖) 스킬 목록을 교체한다. 비어 있으면 기존 목록 유지.
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
protected:
    virtual void BeginPlay() override;
    virtual void OnDead(const TObjectPtr<class AR1Character> Attacker) override;
	virtual void AddCharacterAbility() override;
	virtual void OnHealthChanged(float Ratio, bool bIsDamage) override;

	// Ratio가 다음 페이즈 임계값 이하이면 진입한다. 한 번에 여러 임계값을 넘으면 순서대로 모두 진입한다.
	void AdvancePhasesForRatio(float Ratio);

	// 페이즈 하나에 실제로 진입 (스킬 부여, 목록 교체, 격노 GE, 몽타주, 블랙보드).
	void EnterPhase(int32 PhaseIndex);

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

	// 런타임 스킬 목록. BeginPlay에서 Default/AdditionalSkillAbilities로 초기화된다.
	UPROPERTY(Transient)
	TArray<TSubclassOf<class UGameplayAbility>> ActiveDefaultSkills;

	UPROPERTY(Transient)
	TArray<TSubclassOf<class UGameplayAbility>> ActiveAdditionalSkills;

	// 이미 ASC에 부여한 어빌리티 클래스. 페이즈 진입 시 중복 부여를 막는다.
	UPROPERTY(Transient)
	TSet<TSubclassOf<class UGameplayAbility>> GrantedAbilityClasses;
};
