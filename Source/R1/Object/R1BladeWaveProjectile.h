#pragma once

#include "CoreMinimal.h"
#include "Object/R1Projectile.h"
#include "R1BladeWaveProjectile.generated.h"

/**
 * 블레이드 웨이브 검기 투사체: 파괴되지 않고 관통하며, 캐릭터마다 1회씩만 피해를 준다.
 * 속도/수명/데미지 스펙은 스폰 직후 어빌리티가 세팅한다.
 */
UCLASS()
class R1_API AR1BladeWaveProjectile : public AR1Projectile
{
	GENERATED_BODY()

public:
	// 차지 비율에 따른 균등 스케일 (충돌 구체 + 비주얼). 스폰 직후 1회 호출.
	void SetChargeScale(float InScale);

protected:
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	// 관통 중복 타격 방지용 기록
	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActors;
};
