


#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "Character/R1Character.h"
#include "R1GameplayTags.h"

UR1GameplayAbility::UR1GameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 빙결 상태에서는 모든 어빌리티 활성화 차단 (플레이어/몬스터 공통 — 모든 GA가 이 클래스를 상속)
	ActivationBlockedTags.AddTag(R1GameplayTags::Character_State_Frozen);
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
