


#include "System/R1EquipmentManagerComponent.h"
#include "R1LogChannels.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "R1GameplayTags.h"
#include "Data/R1ItemAssetData.h"
#include "Character/R1Player.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Object/R1WeaponActor.h"
// Sets default values for this component's properties
UR1EquipmentManagerComponent::UR1EquipmentManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UR1EquipmentManagerComponent::EquipItem(ER1EquipmentSlot EquipSlot, UR1ItemAssetData* ItemData)
{
	if (!ASC)
	{
		ASC = Cast<UR1AbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()));
	}

	if (!ASC || !ItemData) return;

	if (EquippedHandlesMap.Contains(EquipSlot))
	{
		UnEquipItem(EquipSlot);
	}

	// 💡 무기를 장착할 때는 기존의 기본 공격(Fist 등)을 제거합니다.
	if (EquipSlot == ER1EquipmentSlot::Weapon && DefaultAttackAbilityHandle.IsValid())
	{
		ASC->ClearAbility(DefaultAttackAbilityHandle);
		DefaultAttackAbilityHandle = FGameplayAbilitySpecHandle();
	}

	// 무기 포즈 타입은 기본 공격 핸들 상태와 무관하게 항상 갱신
	if (EquipSlot == ER1EquipmentSlot::Weapon)
	{
		UpdateWeaponState(ItemData->WeaponType);
	}

	FR1EquipmentActiveHandles NewHandles;

	for (const TSubclassOf<UR1GameplayAbility>& AbilityClass : ItemData->GrantedAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, GetOwner());
			FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);

			NewHandles.AbilityHandles.Add(Handle);

			if (AbilityClass->GetDefaultObject<UR1GameplayAbility>()->GetSkillType() == ER1SkillType::Active)
			{
				AutoAssignToEmptySlot(Handle);
			}
		}
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : ItemData->GrantedEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddInstigator(GetOwner(), GetOwner());

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.0f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveGEHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				NewHandles.EffectHandles.Add(ActiveGEHandle);
			}
		}
	}

	if (ItemData->EquipStatEffect)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddInstigator(GetOwner(), GetOwner());

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ItemData->EquipStatEffect, 1.0f, ContextHandle);

		if (SpecHandle.IsValid())
		{
			static TArray<FGameplayTag> AddStatTags = {
				R1GameplayTags::Data_Attribute_WeaponDamage,
				R1GameplayTags::Data_Attribute_EquipDefence,
				R1GameplayTags::Data_Attribute_MaxHealth,
				R1GameplayTags::Data_Attribute_MaxMana,
				R1GameplayTags::Data_Attribute_MoveSpeed
			};
			static TArray<FGameplayTag> MultiplyStatTags = {
				R1GameplayTags::Data_Attribute_AttackSpeed,
				R1GameplayTags::Data_Attribute_HealthRegeneration,
				R1GameplayTags::Data_Attribute_ManaRegeneration,
				R1GameplayTags::Data_Attribute_DamageMultiplier,
				R1GameplayTags::Data_Attribute_DefenceMultiplier
			};

			for (const FGameplayTag& Tag : AddStatTags)
			{
				float StatValue = 0.0f;
				if (const float* FoundValue = ItemData->StatModifiers.Find(Tag))
				{
					StatValue = *FoundValue;
				}
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(Tag, StatValue);
			}

			for (const FGameplayTag& Tag : MultiplyStatTags)
			{
				float StatValue = 1.0f;
				if (const float* FoundValue = ItemData->StatModifiers.Find(Tag))
				{
					StatValue = *FoundValue;
				}
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(Tag, StatValue);
			}

			FActiveGameplayEffectHandle ActiveGEHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			NewHandles.EffectHandles.Add(ActiveGEHandle);
		}
	}

	EquippedHandlesMap.FindOrAdd(EquipSlot, NewHandles);
	EquippedItemsMap.Add(EquipSlot, ItemData);

	AR1Player* PlayerChar = Cast<AR1Player>(GetOwner());
	if (PlayerChar && PlayerChar->GetMesh() && (ItemData->EquipSlots[0] == ER1EquipmentSlot::Weapon))
	{
		// 방어적 정리: 같은 슬롯에 무기 액터가 남아 있으면 파괴 (세이브 로드 재장착 등에서 누수 방지)
		if (TObjectPtr<AR1WeaponActor>* ExistingActor = EquippedWeaponActorsMap.Find(EquipSlot))
		{
			if (*ExistingActor)
			{
				(*ExistingActor)->Destroy();
			}
			EquippedWeaponActorsMap.Remove(EquipSlot);
		}

		UClass* LoadedWeaponActorClass = ItemData->WeaponActorClass.LoadSynchronous();
		if (LoadedWeaponActorClass)
		{
			// [신규 경로] BP 무기 액터 스폰 — 메시/VFX 배치는 BP 안에서 완결
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AR1WeaponActor* WeaponActor = GetWorld()->SpawnActor<AR1WeaponActor>(LoadedWeaponActorClass, SpawnParams);
			if (WeaponActor)
			{
				WeaponActor->AttachToComponent(PlayerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ItemData->EquipSocketName);
				EquippedWeaponActorsMap.Add(EquipSlot, WeaponActor);
			}
		}
		else if (ItemData->ItemMesh)
		{
			// [폴백 경로] 미이관 무기: 기존 스태틱 메시 + 오라 이펙트

			// 스태틱 메시 컴포넌트를 런타임에 생성합니다.
			UStaticMeshComponent* WeaponMeshComp = NewObject<UStaticMeshComponent>(GetOwner());
			WeaponMeshComp->SetStaticMesh(ItemData->ItemMesh);

			// 콜리전을 끕니다 (무기가 캐릭터를 밀어내는 현상 방지)
			WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// 플레이어 메인 메시의 지정된 소켓에 부착합니다. (SnapToTarget 자동 적용)
			WeaponMeshComp->SetupAttachment(PlayerChar->GetMesh(), ItemData->EquipSocketName);
			WeaponMeshComp->RegisterComponent(); // 씬에 등록하여 렌더링 시작

			// 나중에 장비를 벗을 때 지우기 위해 맵에 기록해 둡니다.
			EquippedMeshesMap.Add(EquipSlot, WeaponMeshComp);

			// 속성 무기라면 무기 메시에 오라 이펙트를 부착합니다.
			if (!ItemData->WeaponAuraVFX.IsNull())
			{
				if (UNiagaraSystem* AuraSystem = ItemData->WeaponAuraVFX.LoadSynchronous())
				{
					UNiagaraComponent* AuraComp = NewObject<UNiagaraComponent>(GetOwner());
					AuraComp->SetAsset(AuraSystem);
					AuraComp->SetupAttachment(WeaponMeshComp);
					AuraComp->RegisterComponent();
					EquippedVFXMap.Add(EquipSlot, AuraComp);
				}
				else
				{
					UE_LOG(LogR1, Warning, TEXT("[%s] WeaponAuraVFX 로드 실패"), *ItemData->ItemName.ToString());
				}
			}
		}
	}

	UE_LOG(LogR1, Warning, TEXT("[%s] 슬롯 장착 완료! 스킬 %d개, 효과 %d개 적용됨"),
		*UEnum::GetValueAsString(EquipSlot), NewHandles.AbilityHandles.Num(), NewHandles.EffectHandles.Num());
}

void UR1EquipmentManagerComponent::UnEquipItem(ER1EquipmentSlot EquipSlot)
{
	if (!ASC) return;

	// 1. 해당 슬롯의 영수증 뭉치를 찾습니다.
	if (FR1EquipmentActiveHandles* FoundHandles = EquippedHandlesMap.Find(EquipSlot))
	{
		// 2. 보관해둔 스킬 영수증을 보고 어빌리티 압수!
		for (const FGameplayAbilitySpecHandle& AbilityHandle : FoundHandles->AbilityHandles)
		{
			for (auto It = SkillSlotsMap.CreateIterator(); It; ++It)
			{
				if (It.Value() == AbilityHandle)
				{
					UE_LOG(LogR1, Warning, TEXT("🗑️ 스킬 단축키 등록 해제: %s 슬롯 비워짐"), *UEnum::GetValueAsString(It.Key()));
					It.RemoveCurrent();
				}
			}
			ASC->ClearAbility(AbilityHandle);
		}

		// 3. 보관해둔 이펙트 영수증을 보고 스탯 버프 압수!
		for (const FActiveGameplayEffectHandle& EffectHandle : FoundHandles->EffectHandles)
		{
			ASC->RemoveActiveGameplayEffect(EffectHandle);
		}

		// 4. Map에서 영수증 파기!
		EquippedHandlesMap.Remove(EquipSlot);
		EquippedItemsMap.Remove(EquipSlot);

		// [신규 경로] 무기 액터 파괴 (부착된 액터는 부모와 함께 파괴되지 않음)
		if (TObjectPtr<AR1WeaponActor>* FoundWeaponActor = EquippedWeaponActorsMap.Find(EquipSlot))
		{
			if (*FoundWeaponActor)
			{
				(*FoundWeaponActor)->Destroy();
			}
			EquippedWeaponActorsMap.Remove(EquipSlot);
		}

		// 속성 이펙트를 무기 메시보다 먼저 파괴합니다 (부착 부모가 사라지기 전).
		if (TObjectPtr<UNiagaraComponent>* FoundVFX = EquippedVFXMap.Find(EquipSlot))
		{
			if (*FoundVFX)
			{
				(*FoundVFX)->DestroyComponent();
			}
			EquippedVFXMap.Remove(EquipSlot);
		}

		if (UStaticMeshComponent** FoundMeshComp = EquippedMeshesMap.Find(EquipSlot))
		{
			if (*FoundMeshComp)
			{
				// 월드에서 모델링을 파괴합니다.
				(*FoundMeshComp)->DestroyComponent();
			}
			// 맵에서 기록을 지웁니다.
			EquippedMeshesMap.Remove(EquipSlot);
		}

		// 💡 무기를 해제했다면 다시 기본 공격을 부여합니다.
		if (EquipSlot == ER1EquipmentSlot::Weapon)
		{
			if (DefaultAttackAbilityClass)
			{
				DefaultAttackAbilityHandle = ASC->GiveAbility(FGameplayAbilitySpec(DefaultAttackAbilityClass, 1));
			}
			UpdateWeaponState(ER1WeaponType::Unarmed);
		}

		UE_LOG(LogR1, Warning, TEXT("[%s] 슬롯 장착 해제 완료! 모든 효과 정상 회수됨"), *UEnum::GetValueAsString(EquipSlot));
	}
}

USoundBase* UR1EquipmentManagerComponent::GetSoundByTag(ER1EquipmentSlot EquipSlot, FGameplayTag AudioTag) const
{
	if (const TObjectPtr<UR1ItemAssetData>* FoundItem = EquippedItemsMap.Find(EquipSlot))
	{
		if (*FoundItem)
		{
			if (const TSoftObjectPtr<USoundBase>* SoundPtr = (*FoundItem)->AudioRoutingMap.Find(AudioTag))
			{
				return SoundPtr->LoadSynchronous();
			}
		}
	}
	return nullptr;
}

void UR1EquipmentManagerComponent::ExecuteSkillSlot(ER1SkillSlot Slot)
{
	if (ASC && SkillSlotsMap.Contains(Slot))
	{
		FGameplayAbilitySpecHandle HandleToExecute = SkillSlotsMap[Slot];
		if (HandleToExecute.IsValid())
		{
			ASC->TryActivateAbility(HandleToExecute);
			return;
		}
	}
	UE_LOG(LogR1, Warning, TEXT("ℹ️ [%s] 슬롯이 비어있습니다."), *UEnum::GetValueAsString(Slot));
}

void UR1EquipmentManagerComponent::AssignSkillToSlot(ER1SkillSlot Slot, FGameplayAbilitySpecHandle AbilityHandle)
{

}


void UR1EquipmentManagerComponent::AutoAssignToEmptySlot(FGameplayAbilitySpecHandle NewHandle)
{
	ER1SkillSlot SlotOrder[] = { ER1SkillSlot::Q, ER1SkillSlot::W, ER1SkillSlot::E, ER1SkillSlot::R };

	for (ER1SkillSlot Slot : SlotOrder)
	{
		// 이 슬롯이 비어있거나, 등록된 핸들이 유효하지 않다면(이미 삭제된 스킬이라면)
		if (!SkillSlotsMap.Contains(Slot) || !SkillSlotsMap[Slot].IsValid())
		{
			// 해당 슬롯에 등록!
			SkillSlotsMap.Add(Slot, NewHandle);
			UE_LOG(LogR1, Warning, TEXT("✅ 스킬 자동 등록 완료! 슬롯: %s"), *UEnum::GetValueAsString(Slot));
			return; // 하나 등록했으면 끝
		}
	}

	UE_LOG(LogR1, Warning, TEXT("⚠️ 스킬 단축키가 모두 꽉 찼습니다! (Q,W,E,R 모두 사용 중)"));
}

void UR1EquipmentManagerComponent::UpdateWeaponState(ER1WeaponType NewWeaponType)
{
	if (AR1Player* Player = Cast<AR1Player>(GetOwner()))
	{
		Player->SetWeaponType(NewWeaponType);
		UE_LOG(LogR1, Log, TEXT("무기 포즈 타입 변경: %s"), *UEnum::GetValueAsString(NewWeaponType));
	}
}

// Called when the game starts
void UR1EquipmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	ASC = Cast<UR1AbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()));

	// 💡 시작할 때 무기가 장착되어 있지 않다면 기본 공격 부여
	if (ASC && !EquippedItemsMap.Contains(ER1EquipmentSlot::Weapon))
	{
		if (DefaultAttackAbilityClass)
		{
			DefaultAttackAbilityHandle = ASC->GiveAbility(FGameplayAbilitySpec(DefaultAttackAbilityClass, 1));
		}
		UpdateWeaponState(ER1WeaponType::Unarmed);
	}
}

void UR1EquipmentManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 부착된 '액터'는 부모(플레이어) 파괴 시 자동 파괴되지 않으므로 여기서 정리
	// (레벨 전환/런 종료 시 무기 액터가 월드에 고아로 남는 누수 방지)
	for (auto& Pair : EquippedWeaponActorsMap)
	{
		if (Pair.Value)
		{
			Pair.Value->Destroy();
		}
	}
	EquippedWeaponActorsMap.Empty();

	Super::EndPlay(EndPlayReason);
}


