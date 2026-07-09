#include "AbilitySystem/Abilities/R1GameplayAbility_Attack.h"
#include "R1LogChannels.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h" // 몽타주 Task
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"  // 이벤트 대기 Task
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "Player/R1PlayerController.h"
#include "Character/R1Player.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/R1Character.h"
#include "R1GameplayTags.h"
#include "System/R1EquipmentManagerComponent.h"
#include "Kismet/GameplayStatics.h"

UR1GameplayAbility_Attack::UR1GameplayAbility_Attack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)	
{
	// ComboIndex가 액티베이션 사이에 유지되도록 액터당 인스턴스 사용
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UR1GameplayAbility_Attack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false) 
	{
		return false;
	}
	return true;
}

void UR1GameplayAbility_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AR1Character* Attacker = Cast<AR1Character>(ActorInfo->AvatarActor);

	// 콤보 섹션이 설정돼 있으면 순환(또는 무작위) 재생, 없으면 기존 단일 스윙
	FName StartSection = NAME_None;
	if (ComboSections.Num() > 0)
	{
		int32 SectionIndex = 0;
		if (bRandomComboSelection)
		{
			if (ComboSections.Num() > 1 && LastComboIndex != INDEX_NONE)
			{
				// 직전 섹션을 제외한 범위에서 뽑고, 그 이상이면 한 칸 밀어 연속 중복 회피
				SectionIndex = FMath::RandRange(0, ComboSections.Num() - 2);
				if (SectionIndex >= LastComboIndex)
				{
					++SectionIndex;
				}
			}
			else
			{
				SectionIndex = FMath::RandRange(0, ComboSections.Num() - 1);
			}
			LastComboIndex = SectionIndex;
		}
		else
		{
			SectionIndex = ComboIndex % ComboSections.Num();
		}
		StartSection = ComboSections[SectionIndex];
	}

	if (PlayAttackMontageAndWaitForEvent(Attacker, AttackEventTag, StartSection))
	{
		if (ComboSections.Num() > 0 && !bRandomComboSelection)
		{
			ComboIndex = (ComboIndex + 1) % ComboSections.Num();
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UR1GameplayAbility_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 몽타주 콜백을 거치지 않고 외부에서 종료되는 경우(무기 교체 시 ClearAbility 등)에도
	// Casting 상태가 남지 않도록 여기서 확정적으로 복구한다. (OnMontageEnded와 중복돼도 무해)
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (AR1Character* Attacker = Cast<AR1Character>(ActorInfo->AvatarActor.Get()))
		{
			if (Attacker->GetCreatureState() == ECreatureState::Casting)
			{
				Attacker->SetCreatureState(ECreatureState::Moving);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UR1GameplayAbility_Attack::OnMontageEnded()
{
	// 애니메이션이 다 끝났으니 어빌리티를 완전히 종료합니다.
	if (CurrentActorInfo == nullptr || CurrentActorInfo->AvatarActor.IsValid() == false)
	{
		return;
	}

	AR1Character* Attacker = Cast<AR1Character>(CurrentActorInfo->AvatarActor.Get());
	if (Attacker)
	{
		if (Attacker->GetCreatureState() == ECreatureState::Dead)
		{
			Attacker->SetCreatureState(ECreatureState::Dead);
			UE_LOG(LogR1, Warning, TEXT("Dead"));
		}
		else
		{
			Attacker->SetCreatureState(ECreatureState::Moving);
			UE_LOG(LogR1, Warning, TEXT("Moving"));
		}
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UR1GameplayAbility_Attack::OnAttackEventReceived(FGameplayEventData Payload)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	AR1Character* SourceCharacter = Cast<AR1Character>(AvatarActor);

	if (!SourceCharacter) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	AActor* TargetActor = nullptr;

	if (DamageEffect && SourceASC)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);
		if (EffectSpecHandle.IsValid() == false || EffectSpecHandle.Data.IsValid() == false)
		{
			return;
		}

		if (AR1PlayerController* PC = Cast<AR1PlayerController>(SourceCharacter->GetController()))
		{
			if (PC->TargetAttackActor)
			{
				TargetActor = PC->TargetAttackActor;

				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

				if (TargetASC)
				{
					SourceASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);

					FGameplayEventData PayloadData;
					PayloadData.Instigator = SourceCharacter;
					PayloadData.Target = TargetActor;

					FGameplayTag HitEventTag = R1GameplayTags::Ability_Attack;

					// 나 자신에게 이벤트를 보내서, 내 몸에 장착된 패시브 GA들이 듣고 반응하게 합니다.
					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(SourceCharacter, HitEventTag, PayloadData);

					// [오디오] 1) 장착 무기 사운드 + GameplayCue, 2) 없으면 SoundToPlay 폴백(맨손 등)
					USoundBase* WeaponSound = nullptr;
					if (AudioTag.IsValid())
					{
						if (AR1Player* Player = Cast<AR1Player>(SourceCharacter))
						{
							if (UR1EquipmentManagerComponent* EquipManager = Player->GetEquipmentComponent())
							{
								WeaponSound = EquipManager->GetSoundByTag(ER1EquipmentSlot::Weapon, AudioTag);
							}
						}
					}

					if (WeaponSound && GameplayCueTag.IsValid())
					{
						FGameplayCueParameters CueParams;
						CueParams.SourceObject = WeaponSound;
						CueParams.Instigator = SourceCharacter;


						FVector StartLoc = SourceCharacter->GetActorLocation() + FVector(0, 0, 50.0f); // 명치를 향하도록 Z축 보정
						FVector EndLoc = TargetActor->GetActorLocation() + FVector(0, 0, 50.0f);

						FHitResult HitResult;
						FCollisionQueryParams TraceParams;
						TraceParams.AddIgnoredActor(SourceCharacter);

						// 공격자의 명치에서 타겟의 명치로 보이지 않는 선을 긋습니다.
						bool bHit = SourceCharacter->GetWorld()->LineTraceSingleByChannel(
							HitResult, StartLoc, EndLoc, ECC_Visibility, TraceParams);

						if (bHit)
						{
							// 캡슐(피부)에 맞았다면 그 정확한 표면 지점과 각도를 사용합니다.
							CueParams.Location = HitResult.ImpactPoint;
							CueParams.Normal = HitResult.ImpactNormal;
						}
						else
						{
							// 만약 장애물 등으로 빗나갔다면(예외 상황) 기본 위치로 세팅
							CueParams.Location = TargetActor->GetActorLocation() + FVector(0, 0, 50.0f);
							CueParams.Normal = (StartLoc - EndLoc).GetSafeNormal();
						}

						SourceASC->ExecuteGameplayCue(GameplayCueTag, CueParams);
					}
					else if (SoundToPlay)
					{
						UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, TargetActor->GetActorLocation());
					}
				}
			}
			else
			{
				UE_LOG(LogR1, Warning, TEXT("Player Attacked, But TargetActor is NULL!"));
			}
		}
	}
}
