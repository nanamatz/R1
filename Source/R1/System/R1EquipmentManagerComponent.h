

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "DataTable/R1ItemDataRow.h"
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
	void EquipItem(ER1EquipmentSlot EquipSlot, const FR1ItemDataRow& ItemData);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UnEquipItem(ER1EquipmentSlot EquipSlot);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// 이 컴포넌트가 부착된 캐릭터의 ASC 캐싱
	UPROPERTY()
	TObjectPtr<class UAbilitySystemComponent> ASC;

	// 💡 [핵심] 현재 장착 중인 장비들의 영수증을 슬롯별로 관리하는 맵(Map)
	UPROPERTY()
	TMap<ER1EquipmentSlot, FR1EquipmentActiveHandles> EquippedHandlesMap;
};
