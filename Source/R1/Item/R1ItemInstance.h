

#pragma once

#include "CoreMinimal.h"
#include "R1Define.h"
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

public:
	void Init(int32 InItemID, FIntPoint InItemSize = FIntPoint(1, 1), ER1EquipmentSlot InEquipSlot = ER1EquipmentSlot::None);

public:
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 ItemID = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	EItemRarity ItemRarity = EItemRarity::Junk;

	// 💡 추가: 아이템이 인벤토리에서 차지하는 가로/세로 칸 수 (예: 무기면 X=2, Y=3)
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	FIntPoint ItemSize = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UTexture2D> ItemIcon;

	// 💡 추가: 이 아이템이 장착될 수 있는 장비 슬롯 부위
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	ER1EquipmentSlot EquipSlot = ER1EquipmentSlot::None;
};
