

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "R1Define.h"
#include "R1EquipmentManagerComponent.generated.h"

USTRUCT(BlueprintType)
struct FR1EquipmentActiveHandles
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> EffectHandles;

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilityHandles;

	void Clear()
	{
		EffectHandles.Empty();
		AbilityHandles.Empty();
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class R1_API UR1EquipmentManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UR1EquipmentManagerComponent();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipItem(ER1EquipmentSlot EquipSlot, class UR1ItemAssetData* ItemData);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UnEquipItem(ER1EquipmentSlot EquipSlot);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	USoundBase* GetSoundByTag(ER1EquipmentSlot EquipSlot, FGameplayTag AudioTag) const;

public:
	// Q, W, E, R 단축키를 눌렀을 때 실행할 함수
	void ExecuteSkillSlot(ER1SkillSlot Slot);

	// 플레이어가 UI에서 직접 단축키를 변경할 때 쓸 함수
	UFUNCTION(BlueprintCallable)
	void AssignSkillToSlot(ER1SkillSlot Slot, FGameplayAbilitySpecHandle AbilityHandle);

public:
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TSubclassOf<class UGameplayAbility> DefaultAttackAbilityClass;

private:
	FGameplayAbilitySpecHandle DefaultAttackAbilityHandle;

	void UpdateWeaponState(bool bIsEquipped);

	UPROPERTY()
	TMap<ER1SkillSlot, FGameplayAbilitySpecHandle> SkillSlotsMap;

	// 빈 단축키를 찾아서 순서대로 꽂아주는 헬퍼 함수
	void AutoAssignToEmptySlot(FGameplayAbilitySpecHandle NewHandle);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// 이 컴포넌트가 부착된 캐릭터의 ASC 캐싱
	UPROPERTY()
	TObjectPtr<class UR1AbilitySystemComponent> ASC;

	// 💡 [핵심] 현재 장착 중인 장비들의 영수증을 슬롯별로 관리하는 맵(Map)
	UPROPERTY()
	TMap<ER1EquipmentSlot, FR1EquipmentActiveHandles> EquippedHandlesMap;

	UPROPERTY()
	TMap<ER1EquipmentSlot, class UStaticMeshComponent*> EquippedMeshesMap;

	UPROPERTY()
	TMap<ER1EquipmentSlot, TObjectPtr<class UR1ItemAssetData>> EquippedItemsMap;
};
