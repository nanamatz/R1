


#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "Character/R1Character.h"

UR1GameplayAbility::UR1GameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UR1GameplayAbility::PlayAttackMontageAndWaitForEvent(AR1Character* Attacker, const FGameplayTag& InAttackEventTag, FName MontageStartSection)
{
	if (Attacker == nullptr || MontageToPlay == nullptr)
	{
		return false;
	}

	Attacker->SetCreatureState(ECreatureState::Casting);

	// GAS에서 현재 AttackSpeed 스탯 값을 읽어 몽타주 재생 배속으로 사용합니다.
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

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		AttackRate,
		MontageStartSection,
		false
	);

	MontageTask->OnCompleted.AddDynamic(this, &UR1GameplayAbility::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UR1GameplayAbility::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UR1GameplayAbility::OnMontageEnded);
	MontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		InAttackEventTag,
		nullptr,
		true,
		false
	);
	WaitEventTask->EventReceived.AddDynamic(this, &UR1GameplayAbility::OnAttackEventReceived);
	WaitEventTask->ReadyForActivation();

	return true;
}

void UR1GameplayAbility::OnMontageEnded()
{
	// 기본 구현 없음 — 서브클래스에서 override.
}

void UR1GameplayAbility::OnAttackEventReceived(FGameplayEventData Payload)
{
	// 기본 구현 없음 — 서브클래스에서 override.
}
