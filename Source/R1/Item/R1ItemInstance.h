

#pragma once

#include "CoreMinimal.h"
#include "R1Define.h"
#include "Data/R1ItemAssetData.h"
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
	// 💡 이제 복잡한 매개변수나 데이터 테이블 ID 대신, 데이터 에셋(UR1ItemAssetData)을 직접 받아 초기화합니다.
void Init(class UR1ItemAssetData* InItemData, EItemRarity InRarity = EItemRarity::Common); 

public:
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TObjectPtr<class UR1ItemAssetData> ItemData;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	EItemRarity ItemRarity = EItemRarity::Common;

public:
	UFUNCTION(BlueprintCallable, Category = "Item")
	UR1ItemAssetData* GetItemData() const { return ItemData; }

	// 2. 사이즈 반환 (안전하게 조건문 유지)
	UFUNCTION(BlueprintCallable, Category = "Item")
	FIntPoint GetItemSize() const { return ItemData ? ItemData->ItemSize : FIntPoint(1, 1); }

	// 3. 🌟 배열(TArray) 반환 시 주의점: nullptr 대신 빈 배열 'TArray<ER1EquipmentSlot>()'을 반환해야 합니다!
	UFUNCTION(BlueprintCallable, Category = "Item")
	TArray<ER1EquipmentSlot> GetEquipSlot() const { return ItemData ? ItemData->EquipSlots : TArray<ER1EquipmentSlot>(); }

	UFUNCTION(BlueprintCallable, Category = "Item")
	UTexture2D* GetItemIcon() const { return ItemData ? ItemData->ItemIcon : nullptr; }
};
