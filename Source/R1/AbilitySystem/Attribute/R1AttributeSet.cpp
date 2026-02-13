


#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Character/R1Character.h"

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
	InitAttackAngle(120.f);
	InitHealthRegeneration(1.f);
	InitManaRegeneration(1.f);
	
}

void UR1AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if(Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		//AR1Character* Character = Cast<AR1Character>(GetOwningActor()); 이 한 줄로도 되는지 한 번 테스트 해봐야 함.
		//// [디버깅 코드 시작]
		//AActor* SourceActor = Data.EffectSpec.GetContext().GetOriginalInstigator(); // 때린 사람
		//AActor* TargetActor = Data.Target.GetAvatarActor(); // 맞은 사람
		//float DamageAmount = Data.EvaluatedData.Magnitude; // 들어온 데미지 양 (음수일 수 있음)

		//UE_LOG(LogTemp, Warning, TEXT("[Damage Debug] %s -> %s에게 데미지: %f"),
		//	*GetNameSafe(SourceActor),
		//	*GetNameSafe(TargetActor),
		//	FMath::Abs(DamageAmount)); // 깎인 양이므로 절대값
		//// [디버깅 코드 끝]

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
					//Character->SetCreatureState(ECreatureState::Dead);
					Character->OnDead(Cast<AR1Character>(Attacker));
				}
			}
		}
	}

	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		AActor* AvatarActor = Data.Target.GetAvatarActor();
		AR1Character* Player = Cast<AR1Character>(AvatarActor);
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

	if (Attribute == GetHealthAttribute())
	{
		float CurrentMaxHealth = GetMaxHealth();
		NewValue = FMath::Clamp(NewValue, 0.0f, CurrentMaxHealth);
	}
}

