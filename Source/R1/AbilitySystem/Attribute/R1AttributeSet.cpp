


#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Player/R1PlayerState.h"
#include "Character/R1Character.h"
#include "Character/R1Player.h"

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
	InitExp(0.f);
	InitMaxExp(10.f);
	InitAggroRange(300.f);
	InitXp(10.f);
	InitLevel(1.f);

}

void UR1AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if(Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
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

	if (Data.EvaluatedData.Attribute == GetExpAttribute())
	{
		// 1. 방금 얻은 경험치량 (GE가 더해준 값)
		float GainedExp = Data.EvaluatedData.Magnitude;

		// 2. 현재 누적된 총 경험치
		float CurExp = GetExp();

		AR1PlayerState* PS = Cast<AR1PlayerState>(GetOwningActor());
		if (PS)
		{
			float Ratio = static_cast<float>(GetExp()) / GetMaxExp();
			PS->OnExpChanged.Broadcast(Ratio); // Broadcast로 수정 (델리게이트 이름 맞추기)
		}
		if (PS->PlayerStatTable)
		{
			float CurrentExp = GetExp();
			float CurrentLevel = GetLevel();

			FRealCurve* MaxExpCurve = PS->PlayerStatTable->FindCurve(FName("MaxExp"), TEXT(""));
			if (!MaxExpCurve)
			{
				UE_LOG(LogTemp, Error, TEXT("Can't find CurveTable Row Named : Max Exp!"));
				return;
			}
			float MaxExpForNextLevel = MaxExpCurve->Eval(CurrentLevel);
			if (MaxExpForNextLevel <= KINDA_SMALL_NUMBER)
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid MaxExp curve value at level %.1f."), CurrentLevel);
				return;
			}

			// 4. 레벨업 루프 (한 번에 여러 레벨이 오를 수도 있으므로 while 사용)
			while (CurrentExp >= MaxExpForNextLevel)
			{
				// 경험치 초과분 계산 및 레벨 1 증가
				CurrentExp -= MaxExpForNextLevel;
				CurrentLevel += 1.0f;

				UE_LOG(LogTemp, Warning, TEXT("Level UP!!!"));

				SetLevel(CurrentLevel);
				SetExp(CurrentExp);

				// 다음 레벨업에 필요한 경험치로 갱신 (루프 계속 진행을 위함)
				MaxExpForNextLevel = MaxExpCurve->Eval(CurrentLevel);
				UE_LOG(LogTemp, Warning, TEXT("Max Exp : %f"), MaxExpForNextLevel);
				SetMaxExp(MaxExpForNextLevel);
				if (MaxExpForNextLevel <= KINDA_SMALL_NUMBER)
				{
					UE_LOG(LogTemp, Error, TEXT("Invalid MaxExp curve value at level %.1f."), CurrentLevel);
					break;
				}
				//레벨 업 후 UI 갱신
				if (PS)
				{
					float Ratio = static_cast<float>(GetExp()) / GetMaxExp();
					PS->OnExpChanged.Broadcast(Ratio);
				}

				// 예: PS->OnLevelUp(); (파티클 재생, 스탯 상승 처리 등)
			}
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
	if (Attribute == GetManaAttribute())
	{
		float CurrentMaxMana = GetMaxMana();
		NewValue = FMath::Clamp(NewValue, 0.0f, CurrentMaxMana);
	}

}

