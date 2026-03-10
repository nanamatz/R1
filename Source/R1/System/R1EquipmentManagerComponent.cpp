


#include "System/R1EquipmentManagerComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "R1GameplayTags.h"

// Sets default values for this component's properties
UR1EquipmentManagerComponent::UR1EquipmentManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UR1EquipmentManagerComponent::EquipItem(ER1EquipmentSlot EquipSlot, const FR1ItemDataRow& ItemData)
{
	if (!ASC)
	{
		ASC = Cast<UR1AbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()));
	}
	if (!ASC) return;

	// 1. 이미 해당 슬롯에 무언가 장착되어 있다면 먼저 벗깁니다!
	if (EquippedHandlesMap.Contains(EquipSlot))
	{
		UnEquipItem(EquipSlot);
	}

	FR1EquipmentActiveHandles NewHandles;

	for (const TSubclassOf<UR1GameplayAbility>& AbilityClass : ItemData.GrantedAbilities)
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

	for (const TSubclassOf<UGameplayEffect>& EffectClass : ItemData.GrantedEffects)
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

	if (ItemData.EquipStatEffect)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddInstigator(GetOwner(), GetOwner());

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ItemData.EquipStatEffect, 1.0f, ContextHandle);

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

				if (const float* FoundValue = ItemData.StatModifiers.Find(Tag))
				{
					StatValue = *FoundValue;
				}
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(Tag, StatValue);
			}

			for (const FGameplayTag& Tag : MultiplyStatTags)
			{
				float StatValue = 1.0f; 

				if (const float* FoundValue = ItemData.StatModifiers.Find(Tag))
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

	UE_LOG(LogTemp, Warning, TEXT("[%s] 슬롯 장착 완료! 스킬 %d개, 효과 %d개 적용됨"),
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
					UE_LOG(LogTemp, Warning, TEXT("🗑️ 스킬 단축키 등록 해제: %s 슬롯 비워짐"), *UEnum::GetValueAsString(It.Key()));
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

		UE_LOG(LogTemp, Warning, TEXT("[%s] 슬롯 장착 해제 완료! 모든 효과 정상 회수됨"), *UEnum::GetValueAsString(EquipSlot));
	}
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
	UE_LOG(LogTemp, Warning, TEXT("ℹ️ [%s] 슬롯이 비어있습니다."), *UEnum::GetValueAsString(Slot));
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
			UE_LOG(LogTemp, Warning, TEXT("✅ 스킬 자동 등록 완료! 슬롯: %s"), *UEnum::GetValueAsString(Slot));
			return; // 하나 등록했으면 끝
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("⚠️ 스킬 단축키가 모두 꽉 찼습니다! (Q,W,E,R 모두 사용 중)"));
}

// Called when the game starts
void UR1EquipmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	ASC = Cast<UR1AbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()));
	
}


