


#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Character/R1Character.h"
#include "Character/R1Player.h"
#include "Character/R1Monster.h"

UR1AttributeSet::UR1AttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitMana(100.f);
	InitMaxMana(100.f);
	InitBaseDamage(10.f);
	InitBaseDefence(5.f);
	InitAttackRange(200.f);
	InitAttackRadius(50.f);
	InitHealthRegeneration(1.f);
	InitManaRegeneration(1.f);

}

void UR1AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if(Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		//AR1Character* Character = Cast<AR1Character>(GetOwningActor()); 이 한 줄로도 되는지 한 번 테스트 해봐야 함.

		AActor* AvatarActor = Data.Target.GetAvatarActor();
		AR1Character* Character = Cast<AR1Character>(AvatarActor);
		if (Character)
		{
			float Ratio = static_cast<float>(GetHealth()) / GetMaxHealth();
			Character->OnHealthChanged(Ratio);

			if (GetHealth() <= 0.0f)
			{
				AActor* Attacker = Data.EffectSpec.GetContext().GetInstigator(); // 시전자 (Pawn)

				if (Character->GetCreatureState() != ECreatureState::Dead)
				{
					Character->SetCreatureState(ECreatureState::Dead);
					Character->OnDead(Cast<AR1Character>(Attacker));
				}
			}
		}
	}

	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		AActor* AvatarActor = Data.Target.GetAvatarActor();
		AR1Player* Player = Cast<AR1Player>(AvatarActor);
		if (Player)
		{
			float Ratio = static_cast<float>(GetMana()) / GetMaxMana();
			Player->OnHealthChanged(Ratio);
		}
	}


}

void UR1AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 1. 지금 변경되려는 속성이 'Health'인지 확인
	if (Attribute == GetHealthAttribute())
	{
		//AActor* AvatarActor = GetOwningAbilitySystemComponent()->GetAvatarActor();
		//AR1Character* Character = Cast<AR1Character>(AvatarActor);
		//if (Character && Character->GetCreatureState() == ECreatureState::Dead)
		//{
		//	if (NewValue > GetHealth())
		//	{
		//		NewValue = GetHealth();
		//	}
		//}

		// 2. 현재 MaxHealth 값을 가져옴
		float CurrentMaxHealth = GetMaxHealth();

		// 3. 들어오는 값(NewValue)을 0 ~ MaxHealth 사이로 가둠 (Clamping)
		// FMath::Clamp(검사할 값, 최소값, 최대값)
		NewValue = FMath::Clamp(NewValue, 0.0f, CurrentMaxHealth);
	}
}

//void UR1AttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
//{
//	Super::PostAttributeChange(Attribute, OldValue, NewValue);
//
//	if (Attribute == GetHealthAttribute())
//	{
//		// 여기서도 UI 업데이트를 요청할 수 있습니다.
//		// 하지만 보통은 PostGameplayEffectExecute와 역할이 나뉩니다.
//		// 여기서는 주로 Clamping만 합니다.
//		float ClampedValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
//	}
//}
