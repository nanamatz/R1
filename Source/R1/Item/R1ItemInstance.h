

#pragma once

#include "CoreMinimal.h"
#include "R1Define.h"
#include "DataTable/R1ItemDataRow.h"
#include "GameplayEffectTypes.h" 
#include "GameplayAbilitySpecHandle.h"
#include "UObject/NoExportTypes.h"
#include "R1ItemInstance.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class R1_API UR1ItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UR1ItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

//public:
//	void Init(int32 InItemID, FIntPoint InItemSize = FIntPoint(1, 1), ER1EquipmentSlot InEquipSlot = ER1EquipmentSlot::None);

public:
	// 💡 이제 복잡한 매개변수 없이 ItemID 하나만 받아서, 데이터 테이블을 뒤져 스스로 세팅하도록 변경합니다.
	void Init(int32 InItemID, UDataTable* InDataTable);

public:
	// 아이템 고유 ID (데이터 테이블의 Row를 찾거나 세이브/로드할 때 사용)
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 ItemID = 0;

	// 인게임에서 변할 수 있는 데이터 (예: 드랍 시 무작위로 정해지는 희귀도, 강화 수치 등)
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	EItemRarity ItemRarity = EItemRarity::Junk;

	// 💡 핵심: 데이터 테이블에서 긁어온 "변하지 않는 원본 스펙"
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	FR1ItemDataRow ItemData;

	// 💡 GAS 장착 해제용 영수증 (장착 해제 시 스킬/스탯을 회수하기 위해 저장)
	FGameplayAbilitySpecHandle GrantedAbilityHandle;
	FActiveGameplayEffectHandle GrantedEffectHandle;

public:
	// 💡 기존 코드 호환성을 위한 Getter 함수들 
	// (기존 UI 코드에서 Instance->ItemSize 등으로 접근하던 것을 Instance->GetItemSize() 로만 바꿔주면 끝납니다)

	UFUNCTION(BlueprintCallable, Category = "Item")
	FIntPoint GetItemSize() const { return ItemData.ItemSize; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	ER1EquipmentSlot GetEquipSlot() const { return ItemData.EquipSlot; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	UTexture2D* GetItemIcon() const { return ItemData.ItemIcon; }
};
