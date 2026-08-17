

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "R1GameplayAbility_Attack.generated.h"

class AR1Character;
/**
 * 
 */
UCLASS()
class R1_API UR1GameplayAbility_Attack : public UR1GameplayAbility
{
	GENERATED_BODY()
	
public:
	UR1GameplayAbility_Attack(const FObjectInitializer& ObjectInitializer);

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Data")
	FName SkillID = FName("Attack");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UGameplayEffect> DamageEffect;

	// [설정] 애니메이션에서 보낼 이벤트 태그 (예: Event.Montage.Hit)
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Damage")
	FGameplayTag AttackEventTag;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Audio")
	FGameplayTag AudioTag;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Audio")
	FGameplayTag GameplayCueTag;

	// [설정] 콤보 몽타주 섹션 목록. 비어 있으면 기존 단일 스윙(섹션 지정 없이 재생).
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combo")
	TArray<FName> ComboSections;

	// [설정] true면 콤보 섹션을 순서 순환 대신 매번 무작위로 선택 (직전 섹션 연속 중복 없음)
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combo")
	bool bRandomComboSelection = false;

	// [설정] 한 섹션 안에 타격 애님이 2개 이상일 때 true로 켠다.
	// 켜면 Event.Montage.Attack 노티파이가 올 때마다 매번 OnAttackEventReceived가 실행된다.
	// (기본 false = 액티베이션당 첫 노티파이 1회만 — 기존 동작)
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combo")
	bool bAllowMultipleAttackEvents = false;

	// 무작위 모드에서 직전에 재생한 섹션 인덱스 (연속 중복 방지용)
	UPROPERTY(Transient)
	int32 LastComboIndex = INDEX_NONE;

	// 다음에 재생할 콤보 섹션 인덱스 (InstancedPerActor라 액티베이션 간 유지)
	UPROPERTY(Transient)
	int32 ComboIndex = 0;

	virtual void OnMontageEnded() override;
	virtual void OnAttackEventReceived(FGameplayEventData Payload) override;
};
