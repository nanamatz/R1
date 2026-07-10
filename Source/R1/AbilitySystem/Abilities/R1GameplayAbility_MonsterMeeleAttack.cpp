#include "AbilitySystem/Abilities/R1GameplayAbility_MonsterMeeleAttack.h"
#include "AbilitySystemComponent.h"
#include "Character/R1Character.h"
#include "Kismet/GameplayStatics.h"
#include "Library/R1AbilitySystemLibrary.h"
#include "R1GameplayTags.h"
#include "NiagaraSystem.h"

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
			TArray<AActor*> DamagedPlayers;
			UR1AbilitySystemLibrary::ApplySectorDamageToPlayers(EffectSpecHandle, SourceCharacter, SourceASC, &DamagedPlayers);

			// [VFX] 피해를 준 각 플레이어 위치에 무기 임팩트 큐 실행 (어빌리티 지정 VFX를 SourceObject로 전달)
			for (AActor* DamagedPlayer : DamagedPlayers)
			{
				FGameplayCueParameters CueParams;
				CueParams.SourceObject = HitImpactVFX;
				CueParams.Instigator = SourceCharacter;
				CueParams.Location = DamagedPlayer->GetActorLocation() + FVector(0, 0, 50.0f); // 명치 높이 보정
				CueParams.Normal = (SourceCharacter->GetActorLocation() - DamagedPlayer->GetActorLocation()).GetSafeNormal();
				SourceASC->ExecuteGameplayCue(R1GameplayTags::GameplayCue_Weapon_Impact, CueParams);
			}
		}
	}
}
