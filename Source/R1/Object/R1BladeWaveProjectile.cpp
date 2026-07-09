#include "Object/R1BladeWaveProjectile.h"
#include "Character/R1Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"

AR1BladeWaveProjectile::AR1BladeWaveProjectile()
{
	// 검기가 날아가는 동안 제자리 회전 (기본: 요 720도/초)
	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = FRotator(0.0f, 720.0f, 0.0f);

	// 베이스가 켠 속도 방향 강제 회전을 끈다 — 켜두면 매 틱 회전을 덮어써 스핀이 안 보인다.
	ProjectileMovement->bRotationFollowsVelocity = false;
}

void AR1BladeWaveProjectile::SetChargeScale(float InScale)
{
	SetActorScale3D(FVector(InScale));
}

void AR1BladeWaveProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 베이스와 달리 Destroy하지 않는다 — 관통하며 대상마다 1회만 피해 적용.
	if (OtherActor == nullptr || OtherActor == GetInstigator() || HitActors.Contains(OtherActor))
	{
		return;
	}

	AR1Character* TargetCharacter = Cast<AR1Character>(OtherActor);
	if (TargetCharacter == nullptr || TargetCharacter->GetCreatureState() == ECreatureState::Dead)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (TargetASC == nullptr || DamageSpecHandle.IsValid() == false)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	if (SourceASC == nullptr)
	{
		return;
	}

	HitActors.Add(OtherActor);
	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
}
