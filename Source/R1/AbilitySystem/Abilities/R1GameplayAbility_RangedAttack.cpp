#include "AbilitySystem/Abilities/R1GameplayAbility_RangedAttack.h"
#include "Character/R1RangerMonster.h"
#include "Object/R1Projectile.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

void UR1GameplayAbility_RangedAttack::OnAttackEventReceived(FGameplayEventData Payload)
{
    AR1RangerMonster* SourceCharacter = Cast<AR1RangerMonster>(GetAvatarActorFromActorInfo());
    if (!SourceCharacter || !SourceCharacter->ProjectileClass) return;

    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, SourceCharacter->GetActorLocation());
    }

    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!SourceASC || !DamageEffect) return;

    // Create the Effect Spec to pass to the projectile
    FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
    EffectContext.AddSourceObject(SourceCharacter);
    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, EffectContext);

    // Spawn Projectile
    FVector MuzzleLocation = SourceCharacter->GetMesh()->GetSocketLocation(SourceCharacter->MuzzleSocketName);
    
    // Direction: For now, use character forward. In a real game, you might want to aim at the target.
    FRotator MuzzleRotation = SourceCharacter->GetActorRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = SourceCharacter;
    SpawnParams.Instigator = SourceCharacter;

    AR1Projectile* Projectile = GetWorld()->SpawnActor<AR1Projectile>(
        SourceCharacter->ProjectileClass, 
        MuzzleLocation, 
        MuzzleRotation, 
        SpawnParams
    );

    if (FiredEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FiredEffect, MuzzleLocation, MuzzleRotation);
    }

    if (Projectile)
    {
        Projectile->DamageSpecHandle = SpecHandle;
    }
}
