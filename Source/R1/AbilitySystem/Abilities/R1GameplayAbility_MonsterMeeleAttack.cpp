#include "AbilitySystem/Abilities/R1GameplayAbility_MonsterMeeleAttack.h"
#include "AbilitySystemComponent.h"
#include "Character/R1Character.h"
#include "Kismet/GameplayStatics.h"
#include "Library/R1AbilitySystemLibrary.h"

UR1GameplayAbility_MonsterMeeleAttack::UR1GameplayAbility_MonsterMeeleAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UR1GameplayAbility_MonsterMeeleAttack::OnAttackEventReceived(FGameplayEventData Payload)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	AR1Character* SourceCharacter = Cast<AR1Character>(AvatarActor);

	if (!SourceCharacter) return;

	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, SourceCharacter->GetActorLocation());
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	if (DamageEffect && SourceASC)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);
		if (EffectSpecHandle.IsValid() && EffectSpecHandle.Data.IsValid())
		{
			UR1AbilitySystemLibrary::ApplySectorDamageToPlayers(EffectSpecHandle, SourceCharacter, SourceASC);
		}
	}
}
