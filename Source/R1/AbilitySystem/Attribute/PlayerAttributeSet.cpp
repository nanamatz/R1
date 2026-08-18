


#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "R1LogChannels.h"
#include "GameplayEffectExtension.h"
#include "Player/R1PlayerState.h"
#include "Character/R1Character.h"
#include "Character/R1Player.h"
#include "R1GameplayTags.h"

UPlayerAttributeSet::UPlayerAttributeSet()
{
	InitManaRegeneration(0.f);
	InitMana(0.f);
	InitMaxMana(0.f);
	InitExp(0.f);
	InitMaxExp(0.f);
	InitLevel(1.f);
	InitHonor(0.f);
	InitRevive(0.f);
	InitExtraGold(0.f);
	InitExtraExp(0.f);
	InitLuck(0.f);
}


void UPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetManaAttribute())
	{
		float CurrentMaxMana = GetMaxMana();
		NewValue = FMath::Clamp(NewValue, 0.0f, CurrentMaxMana);
	}
}

void UPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));

		AActor* AvatarActor = Data.Target.GetAvatarActor();
		AR1Player* Player = Cast<AR1Player>(AvatarActor);
		if (Player)
		{
			float Ratio = static_cast<float>(GetMana()) / GetMaxMana();
			Player->OnManaChanged(Ratio);
		}
	}

	if (Data.EvaluatedData.Attribute == GetExpAttribute())
	{
		// 1. 방금 얻은 경험치량 (GE가 더해준 값)
		float GainedExp = Data.EvaluatedData.Magnitude;

		// 메타 업그레이드 ExtraExp는 퍼센트 단위(10 = +10%).
		// GE가 이미 더한 원본 획득량에 비례해 추가분만 얹는다. 아래 레벨업 루프가 갱신된 Exp를 그대로 사용한다.
		const float ExtraExpPercent = FMath::Max(0.0f, GetExtraExp());
		if (ExtraExpPercent > 0.0f && GainedExp > 0.0f)
		{
			SetExp(GetExp() + GainedExp * (ExtraExpPercent / 100.0f));
		}

		// 2. 현재 누적된 총 경험치
		float CurExp = GetExp();

		AR1PlayerState* PS = Cast<AR1PlayerState>(GetOwningActor());
		if (PS)
		{
			float Ratio = static_cast<float>(GetExp()) / GetMaxExp();
			PS->OnExpChanged.Broadcast(Ratio); // Broadcast로 수정 (델리게이트 이름 맞추기)
		}
		// 3. 경험치 합산
		if (PS->PlayerStatTable)
		{
			float CurrentExp = GetExp();
			float CurrentLevel = GetLevel();
			// 테이블 조회 (MaxExp 곡선)
			FRealCurve* MaxExpCurve = PS->PlayerStatTable->FindCurve(FName("MaxExp"), TEXT(""));
			if (!MaxExpCurve)
			{
				UE_LOG(LogR1, Error, TEXT("Can't find CurveTable Row Named : Max Exp!"));
				return;
			}
			// 다음 레벨업에 필요한 최대 경험치 계산
			float MaxExpForNextLevel = MaxExpCurve->Eval(CurrentLevel);

			if (MaxExpForNextLevel <= KINDA_SMALL_NUMBER)
			{
				UE_LOG(LogR1, Error, TEXT("Invalid MaxExp curve value at level %.1f."), CurrentLevel);
				return;
			}

			// 4. 레벨업 루프 (한 번에 여러 레벨이 오를 수도 있으므로 while 사용)
			while (CurrentExp >= MaxExpForNextLevel)
			{
				// 경험치 초과분 계산 및 레벨 1 증가
				CurrentExp -= MaxExpForNextLevel;
				CurrentLevel += 1.0f;

				UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
				if (ASC)
				{
					FGameplayEventData EventData;

					// 사용할 이벤트 태그 (프로젝트에 등록된 태그 사용 권장)
					EventData.EventTag = R1GameplayTags::Ability_LevelUp;
					EventData.Instigator = GetOwningActor(); // PlayerState
					EventData.Target = GetOwningActor();

					// 이벤트를 아바타(자신)에게 보냅니다.
					ASC->HandleGameplayEvent(EventData.EventTag, &EventData);

				}

				UE_LOG(LogR1, Warning, TEXT("Level UP!!!"));

				SetLevel(CurrentLevel);
				SetExp(CurrentExp);

				// 다음 레벨업에 필요한 경험치로 갱신 (루프 계속 진행을 위함)
				MaxExpForNextLevel = MaxExpCurve->Eval(CurrentLevel);
				UE_LOG(LogR1, Warning, TEXT("Max Exp : %f"), MaxExpForNextLevel);
				SetMaxExp(MaxExpForNextLevel);
				if (MaxExpForNextLevel <= KINDA_SMALL_NUMBER)
				{
					UE_LOG(LogR1, Error, TEXT("Invalid MaxExp curve value at level %.1f."), CurrentLevel);
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


