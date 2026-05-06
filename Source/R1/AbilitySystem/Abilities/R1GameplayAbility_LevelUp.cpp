


#include "AbilitySystem/Abilities/R1GameplayAbility_LevelUp.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/R1Player.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "R1GameplayAbility_LevelUp.h"

UR1GameplayAbility_LevelUp::UR1GameplayAbility_LevelUp(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
}


void UR1GameplayAbility_LevelUp::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AR1Player* R1Player = Cast<AR1Player>(ActorInfo->AvatarActor);

	if (R1Player)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(R1Player);
		if (ASC && LevelUpRecoveryEffect) 
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddInstigator(R1Player, R1Player);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(LevelUpRecoveryEffect, 1.0f, ContextHandle);

			// 4. 플레이어에게 GE를 적용합니다!
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
		if (LevelUpParticleEffectClass && LevelUpSound)
		{
			FVector SpawnLocation = ActorInfo->AvatarActor->GetActorLocation() + FVector(0.f,0.f,180.f);
			FRotator SpawnRotation = ActorInfo->AvatarActor->GetActorRotation();

			// 연출용 액터를 소환! (액터 내부에서 파티클들이 자동으로 터지게 세팅)
			GetWorld()->SpawnActor<AActor>(LevelUpParticleEffectClass, SpawnLocation, SpawnRotation);

			UGameplayStatics::PlaySound2D(this, LevelUpSound);
		}
	}

	// 레벨업 어빌리티가 끝났음을 알림
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

