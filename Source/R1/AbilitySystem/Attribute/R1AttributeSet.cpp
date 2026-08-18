


#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "R1LogChannels.h"
#include "GameplayEffectExtension.h"
#include "Player/R1PlayerState.h"
#include "Character/R1Character.h"
#include "Character/R1Player.h"
#include "R1GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "System/R1DamageUISubsystem.h"

namespace
{
	// 부활 직후 연타로 다시 죽지 않도록 주는 무적 시간(초).
	constexpr float ReviveInvulnerabilityDuration = 2.0f;

	// 메타 업그레이드 Revive 충전이 남아 있으면 사망을 취소하고 그 자리에서 부활시킨다.
	// 부활했으면 true(= OnDead를 부르지 않는다), 아니면 false.
	bool TryReviveInPlace(AR1Character* Character, UR1AttributeSet* AttributeSet)
	{
		AR1Player* Player = Cast<AR1Player>(Character);
		if (!Player || !AttributeSet)
		{
			return false;
		}

		AR1PlayerState* PS = Player->GetPlayerState<AR1PlayerState>();
		if (!PS || !PS->TryConsumeRevive())
		{
			return false;
		}

		const float RevivedHealth = AttributeSet->GetMaxHealth() * 0.5f;
		AttributeSet->SetHealth(RevivedHealth);

		// 위쪽 OnHealthChanged는 체력 0 기준으로 이미 나갔으므로, 회복된 값으로 UI를 다시 갱신한다.
		Character->OnHealthChanged(RevivedHealth / AttributeSet->GetMaxHealth(), false);

		// R1DamageExecutionCalc가 이 태그를 보고 피해를 무시한다.
		if (UAbilitySystemComponent* ASC = AttributeSet->GetOwningAbilitySystemComponent())
		{
			ASC->AddLooseGameplayTag(R1GameplayTags::Character_State_Invulnerable);

			if (UWorld* World = Character->GetWorld())
			{
				TWeakObjectPtr<UAbilitySystemComponent> WeakASC(ASC);
				FTimerHandle InvulnerabilityTimer;
				World->GetTimerManager().SetTimer(InvulnerabilityTimer, [WeakASC]()
				{
					if (WeakASC.IsValid())
					{
						WeakASC->RemoveLooseGameplayTag(R1GameplayTags::Character_State_Invulnerable);
					}
				}, ReviveInvulnerabilityDuration, false);
			}
		}

		UE_LOG(LogR1, Warning, TEXT("[Revive] 부활! 체력 %.0f 로 복구, %.1f초 무적"), RevivedHealth, ReviveInvulnerabilityDuration);
		return true;
	}
}

UR1AttributeSet::UR1AttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitBaseDamage(10.f);
	InitBaseDefence(5.f);
	InitCriticalHitChance(0.0f);
	InitCriticalHitMultiplier(2.0f);
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
	//else if (Attribute == GetMaxHealthAttribute())
	//{
	//	// 1. 기존 MaxHealth와 변경될 NewValue(새 MaxHealth)를 가져옵니다.
	//	float CurrentMaxHealth = GetMaxHealth();

	//	if (CurrentMaxHealth > 0.0f)
	//	{
	//		// 2. 현재 체력의 '비율'을 구합니다.
	//		float CurrentRatio = GetHealth() / CurrentMaxHealth;

	//		// 3. 새 MaxHealth에 기존 비율을 곱해 새로운 체력 값을 계산합니다.
	//		float NewHealth = NewValue * CurrentRatio;

	//		// 4. ASC를 통해 현재 Health 값을 안전하게 덮어씌웁니다.
	//		if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
	//		{
	//			ASC->SetNumericAttributeBase(GetHealthAttribute(), NewHealth);
	//		}
	//	}
	//}
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
			 UE_LOG(LogR1, Warning, TEXT("이동 속도 변경됨: %f -> %f"), OldValue, NewValue);
		}
	}
}


void UR1AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if(Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		AActor* AvatarActor = Data.Target.GetAvatarActor();
		AR1Character* Character = Cast<AR1Character>(AvatarActor);

		// 하한은 보통 0이지만, 보스가 페이즈 임계값에 있을 때는 그 값이 하한이 된다.
		// (임계값에서 체력을 멈춰 페이즈 연출을 보장하고 페이즈 건너뛰기를 막는다.)
		const float HealthFloor = Character ? Character->GetHealthFloor() : 0.0f;
		SetHealth(FMath::Clamp(GetHealth(), HealthFloor, GetMaxHealth()));

		if (Character)
		{
			float Ratio = static_cast<float>(GetHealth()) / GetMaxHealth();
			bool bIsDamage = (Data.EvaluatedData.Magnitude < 0.0f);
			Character->OnHealthChanged(Ratio, bIsDamage);

			if (bIsDamage && Character->GetCreatureState() != ECreatureState::Dead)
			{
				UE_LOG(LogR1, Error, TEXT("Damage : %f"), Data.EvaluatedData.Magnitude);
				UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();

				// Hit Stop 적용 (때린 놈과 맞은 놈 모두)
				AActor* Instigator = Data.EffectSpec.GetContext().GetEffectCauser();
				if (AR1Character* Attacker = Cast<AR1Character>(Instigator))
				{
					Attacker->ApplyHitStop(0.02f);
				}
				Character->ApplyHitStop(0.03f);

				if (TargetASC)
				{
					FGameplayEventData EventData;
					EventData.EventTag = R1GameplayTags::Event_HitReact;					
					EventData.Instigator = Data.EffectSpec.GetContext().GetEffectCauser(); // 때린 놈
					EventData.Target = Character; // 맞은 놈
					EventData.EventMagnitude = FMath::Abs(Data.EvaluatedData.Magnitude); // 데미지량

					// 아바타에게 "너 맞았어!" 라고 태그 이벤트 전달
					TargetASC->HandleGameplayEvent(EventData.EventTag, &EventData);
				}

				int32 DamageAmount = FMath::FloorToInt(FMath::Abs(Data.EvaluatedData.Magnitude));
    
				FR1DamageInfo DamageInfo;
				DamageInfo.DamageAmount = DamageAmount;
				
				// Check for critical hit tag injected during execution
				if (Data.EffectSpec.GetDynamicAssetTags().HasTagExact(R1GameplayTags::Event_Hit_Critical))
				{
					DamageInfo.DamageType = ER1DamageType::Critical;
				}
				else
				{
					DamageInfo.DamageType = ER1DamageType::Normal;
				}
				
				FVector TargetLoc = Character->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
				if (USkeletalMeshComponent* Mesh = Character->GetMesh())
				{
					if (Mesh->DoesSocketExist(FName("DamageTextSocket")))
					{
						TargetLoc = Mesh->GetSocketLocation(FName("DamageTextSocket"));
					}
				}
				DamageInfo.TargetLocation = TargetLoc;

				if (UWorld* World = Character->GetWorld())
				{
					if (UR1DamageUISubsystem* DamageSS = World->GetSubsystem<UR1DamageUISubsystem>())
					{
						DamageSS->ShowDamageText(DamageInfo);
					}
				}
			}

			if (GetHealth() <= 0.0f)
			{
				AActor* Attacker = Data.EffectSpec.GetContext().GetEffectCauser(); // 시전자 (Pawn)
				
				if (Character->GetCreatureState() != ECreatureState::Dead)
				{
					// 부활 충전이 남아 있으면 사망 처리 자체를 건너뛴다.
					if (!TryReviveInPlace(Character, this))
					{
						Character->OnDead(Cast<AR1Character>(Attacker));
					}
				}
			}
		}
	}
}


