


#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "Character/R1Character.h"
#include "R1GameplayTags.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"

UObject* UR1GameplayAbility::GetHitImpactEffect() const
{
	if (HitImpactVFX)
	{
		return HitImpactVFX;
	}
	return HitImpactCascade;
}

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

	// 몽타주 재생이 즉시 실패하면 OnCancelled → 서브클래스 OnMontageEnded → EndAbility가
	// 이 호출 안에서 동기적으로 실행되어 인스턴스가 가비지로 마킹될 수 있다.
	// 그 상태로 아래에서 AddDynamic을 호출하면 "Unable to bind delegate" ensure가 발생하므로 중단한다.
	if (!IsActive())
	{
		return false;
	}

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
