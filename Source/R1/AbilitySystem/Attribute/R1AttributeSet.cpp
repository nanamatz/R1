


#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Player/R1PlayerState.h"
#include "Character/R1Character.h"
#include "Character/R1Player.h"
#include "R1GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "System/R1DamageUISubsystem.h"

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

			if (Data.EvaluatedData.Magnitude < 0.0f && Character->GetCreatureState() != ECreatureState::Dead)
			{
				UE_LOG(LogTemp, Error, TEXT("Damage : %f"), Data.EvaluatedData.Magnitude);
				UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();
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

				float DamageAmount = FMath::Abs(Data.EvaluatedData.Magnitude);
    
				FR1DamageInfo DamageInfo;
				DamageInfo.DamageAmount = DamageAmount;
				DamageInfo.DamageType = ER1DamageType::Normal;
				
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
					Character->OnDead(Cast<AR1Character>(Attacker));
				}
			}
		}
	}
}


