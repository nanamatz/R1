
#include "AbilitySystem/Abilities/R1GameplayAbility_MonsterComboAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Character/R1Character.h"
#include "Character/R1Player.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "R1GameplayTags.h"
#include "Engine/OverlapResult.h"

UR1GameplayAbility_MonsterComboAttack::UR1GameplayAbility_MonsterComboAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UR1GameplayAbility_MonsterComboAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AR1Character* Attacker = Cast<AR1Character>(ActorInfo->AvatarActor);

	if (Attacker && MontageToPlay)
	{
		Attacker->SetCreatureState(ECreatureState::Casting);
		float AttackRate = 1.0f;

		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			bool bFound = false;
			float CurrentAttackSpeed = ASC->GetGameplayAttributeValue(UR1AttributeSet::GetAttackSpeedAttribute(), bFound);
			if (bFound)
			{
				AttackRate = CurrentAttackSpeed;
			}
		}

		// Combo Section Cycling
		FName StartSectionName = (ComboIndex == 0) ? FName("Combo1") : FName("Combo2");

		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			MontageToPlay,
			AttackRate,
			StartSectionName,
			false
		);

		MontageTask->OnCompleted.AddDynamic(this, &UR1GameplayAbility_MonsterComboAttack::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UR1GameplayAbility_MonsterComboAttack::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UR1GameplayAbility_MonsterComboAttack::OnMontageEnded);

		MontageTask->ReadyForActivation();

		ComboIndex = (ComboIndex + 1) % 2;

		// Wait for Hit Event
		UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			AttackEventTag,
			nullptr,
			true,
			false
		);
		WaitEventTask->EventReceived.AddDynamic(this, &UR1GameplayAbility_MonsterComboAttack::OnAttackEventReceived);
		WaitEventTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UR1GameplayAbility_MonsterComboAttack::OnMontageEnded()
{
	if (CurrentActorInfo == nullptr || CurrentActorInfo->AvatarActor.IsValid() == false)
	{
		return;
	}

	AR1Character* Attacker = Cast<AR1Character>(CurrentActorInfo->AvatarActor.Get());
	if (Attacker)
	{
		if (Attacker->GetCreatureState() != ECreatureState::Dead)
		{
			Attacker->SetCreatureState(ECreatureState::Moving);
		}
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UR1GameplayAbility_MonsterComboAttack::OnAttackEventReceived(FGameplayEventData Payload)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	AR1Character* SourceCharacter = Cast<AR1Character>(AvatarActor);

	if (!SourceCharacter) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	if (DamageEffect && SourceASC)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);
		if (EffectSpecHandle.IsValid() && EffectSpecHandle.Data.IsValid())
		{
			CheckAndApplyDamage_Sector(EffectSpecHandle, SourceCharacter, SourceASC);
		}
	}
}

void UR1GameplayAbility_MonsterComboAttack::CheckAndApplyDamage_Sector(const FGameplayEffectSpecHandle& SpecHandle, AR1Character* SourceCharacter, UAbilitySystemComponent* SourceASC)
{
	if (SourceCharacter == nullptr || SourceASC == nullptr) return;

	UWorld* World = SourceCharacter->GetWorld();
	if (World == nullptr) return;

	float AttackRange = SourceASC->GetNumericAttribute(UR1AttributeSet::GetAttackRangeAttribute());
	float AttackRadius = SourceASC->GetNumericAttribute(UR1AttributeSet::GetAttackRadiusAttribute());
	
	if (AttackRange <= KINDA_SMALL_NUMBER || AttackRadius <= KINDA_SMALL_NUMBER) return;

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(SourceCharacter);

	bool bResult = World->OverlapMultiByChannel(
		OverlapResults,
		SourceCharacter->GetActorLocation(),
		FQuat::Identity,
		ECC_GameTraceChannel2, // Assume ECC_GameTraceChannel2 is used for player detection
		FCollisionShape::MakeSphere(AttackRange),
		Params
	);

	if (!bResult) return;

	float DotThreshold = FMath::Cos(FMath::DegreesToRadians(AttackRadius / 2.0f));
	FVector MyForward = SourceCharacter->GetActorForwardVector();
	TSet<AActor*> ProcessedActors;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* TargetActor = Result.GetActor();
		if (!TargetActor || ProcessedActors.Contains(TargetActor)) continue;

		AR1Player* TargetPlayer = Cast<AR1Player>(TargetActor);
		if (!TargetPlayer) continue;

		FVector DirToTarget = (TargetPlayer->GetActorLocation() - SourceCharacter->GetActorLocation()).GetSafeNormal();
		float DotResult = FVector::DotProduct(MyForward, DirToTarget);

		if (DotResult >= DotThreshold)
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetPlayer);
			if (TargetASC)
			{
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				ProcessedActors.Add(TargetPlayer);

				if (SoundToPlay)
				{
					UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, TargetPlayer->GetActorLocation());
				}
			}
		}
	}
}
