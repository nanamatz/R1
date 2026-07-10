#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility_BossAttackBase.h"
#include "R1GameplayAbility_WaveAttack.generated.h"

class UGameplayEffect;
class UParticleSystem;

UCLASS()
class R1_API UR1GameplayAbility_WaveAttack : public UR1GameplayAbility_BossAttackBase
{
	GENERATED_BODY()

protected:
	virtual void OnAttackEventReceived(FGameplayEventData Payload) override;

	UPROPERTY(EditAnywhere, Category = "Attack")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UParticleSystem> WaveEffect;

	// 타격 임팩트는 레거시 Cascade(ImpactEffect) 대신 베이스의 HitImpactVFX(Niagara) + 무기 임팩트 큐로 재생한다.
};
