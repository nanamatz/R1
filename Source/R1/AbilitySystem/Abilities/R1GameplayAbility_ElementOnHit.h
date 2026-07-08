
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "R1GameplayAbility_ElementOnHit.generated.h"

/**
 * 속성 무기 명중 시 대상에게 GE를 부여하는 이벤트 트리거형 패시브.
 * Ability.Attack 게임플레이 이벤트(공격 어빌리티가 명중 시 재전송)로 자동 활성화되어
 * TriggerEventData->Target에 OnHitEffect를 적용하고 즉시 종료한다.
 * 아이템의 GrantedAbilities에 BP 자식(GA_FireOnHit 등)을 넣어 사용.
 */
UCLASS()
class R1_API UR1GameplayAbility_ElementOnHit : public UR1GameplayAbility
{
	GENERATED_BODY()

public:
	UR1GameplayAbility_ElementOnHit(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 명중한 대상에게 적용할 GE (예: GE_Burning)
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Element")
	TSubclassOf<class UGameplayEffect> OnHitEffect;
};
