
#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "R1GameplayCueNotify_WeaponImpact.generated.h"

class UNiagaraSystem;

/**
 * 무기 명중 임팩트 큐(GameplayCue.Weapon.Impact): SourceObject로 전달된 무기 사운드를 재생하고,
 * 공격자(Instigator)의 장착 무기 DA에서 HitImpactVFX를 찾아 피격 지점(Parameters.Location)에 스폰한다.
 * 무기 VFX가 없으면(맨손/몬스터/구형 DA) DefaultHitVFX 폴백. BP(GCN_WeaponImpact)는 이 클래스로
 * 리페어런팅 후 GameplayCue Tag와 DefaultHitVFX만 설정하면 된다.
 */
UCLASS()
class R1_API UR1GameplayCueNotify_WeaponImpact : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

protected:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

	// 무기 DA에 HitImpactVFX가 없을 때 사용할 기본 임팩트 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "R1|VFX")
	TObjectPtr<UNiagaraSystem> DefaultHitVFX;
};
