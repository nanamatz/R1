


#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Player/R1PlayerState.h"
#include "Character/R1Character.h"
#include "Character/R1Player.h"
#include "GameFramework/CharacterMovementComponent.h"

UR1AttributeSet::UR1AttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitBaseDamage(10.f);
	InitBaseDefence(5.f);
	InitAttackRange(200.f);
	InitAttackRadius(50.f);
	InitHealthRegeneration(1.f);
	InitMoveSpeed(600.f);
	InitAttackSpeed(1.f);
}

void UR1AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		float CurrentMaxHealth = GetMaxHealth();
		NewValue = FMath::Clamp(NewValue, 0.0f, CurrentMaxHealth);
	}
}

void UR1AttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	if (Attribute == GetMoveSpeedAttribute())
	{
		// 아바타 액터(실제 월드에 존재하는 캐릭터)를 가져옵니다.
		AActor* AvatarActor = GetOwningAbilitySystemComponent()->GetAvatarActor();
		AR1Character* Character = Cast<AR1Character>(AvatarActor);

		if (Character && Character->GetCharacterMovement())
		{
			// 캐릭터의 실제 걷기 최고 속도를 GAS의 MoveSpeed 값으로 동기화!
			Character->GetCharacterMovement()->MaxWalkSpeed = NewValue;

			// 로그로 잘 적용되는지 확인해 보세요!
			 UE_LOG(LogTemp, Warning, TEXT("이동 속도 변경됨: %f -> %f"), OldValue, NewValue);
		}
	}
}


void UR1AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if(Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

		AActor* AvatarActor = Data.Target.GetAvatarActor();
		AR1Character* Character = Cast<AR1Character>(AvatarActor);
		if (Character)
		{
			float Ratio = static_cast<float>(GetHealth()) / GetMaxHealth();
			Character->OnHealthChanged(Ratio);

			if (GetHealth() <= 0.0f)
			{
				AActor* Attacker = Data.EffectSpec.GetContext().GetEffectCauser(); // 시전자 (Pawn)
				
				if (Character->GetCreatureState() != ECreatureState::Dead)
				{
					Character->OnDead(Cast<AR1Character>(Attacker));
				}
			}
		}
	}
}


