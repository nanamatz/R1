#include "Object/R1BladeWaveProjectile.h"
#include "Character/R1Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

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
