

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

	// 다음에 재생할 콤보 섹션 인덱스 (InstancedPerActor라 액티베이션 간 유지)
	UPROPERTY(Transient)
	int32 ComboIndex = 0;

	virtual void OnMontageEnded() override;
	virtual void OnAttackEventReceived(FGameplayEventData Payload) override;
};
