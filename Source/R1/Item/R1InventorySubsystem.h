

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "R1Define.h"
#include "R1InventorySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
class UR1ItemInstance;

/**
 * 
 */
UCLASS()
class R1_API UR1InventorySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	const TArray<TObjectPtr<UR1ItemInstance>>& GetGridData() const { return GridData; }

	void AddDefaultItem();

	const TArray<TObjectPtr<UR1ItemInstance>>& GetItems() { return Items; }

	const TMap<ER1EquipmentSlot, TObjectPtr<UR1ItemInstance>>& GetEquippedItems() const { return EquippedItems; }

	FIntPoint GetItemPosition(UR1ItemInstance* Item) const;

	void ClearInventory();

	void LoadItem(int32 ItemID, EItemRarity Rarity, FIntPoint Pos);
	void LoadEquippedItem(int32 ItemID, EItemRarity Rarity, ER1EquipmentSlot Slot);

	// 💡 가장 중요한 핵심 함수: 특정 위치에 아이템을 놓을 수 있는지 검사
	bool CanAddItemAt(const FIntPoint& ItemSize, const FIntPoint& TargetPos, UR1ItemInstance* IgnoreItem = nullptr);

	int32 GetInventoryColumns() const { return 10; } // X 칸 수
	int32 GetInventoryRows() const { return 5; }    // Y 칸 수

public:
	// 아이템을 인벤토리 내에서 다른 좌표로 이동시킵니다 (그리드 데이터 갱신)
	void AddItemToGrid(UR1ItemInstance* Item, const FIntPoint& Pos);
	void RemoveItemFromGrid(UR1ItemInstance* Item, const FIntPoint& Pos);
	void MoveItemInGrid( UR1ItemInstance* Item, FIntPoint OldPos, FIntPoint NewPos);

public:
	// 아이템을 인벤토리에서 장비창으로 이동
	bool EquipItem(UR1ItemInstance* ItemToEquip, ER1EquipmentSlot SpecificSlot = ER1EquipmentSlot::None);

	// 장비를 해제하여 다시 인벤토리로 이동
	bool UnequipItem(ER1EquipmentSlot TargetSlot);

	UPROPERTY(BlueprintAssignable)
	FOnInventoryUpdated OnInventoryUpdated;

protected:
	// 전체 그리드를 1차원 배열로 관리 (크기: Columns * Rows)
	// 비어있으면 nullptr, 누군가 차지하고 있으면 그 아이템의 포인터가 들어감
	UPROPERTY()
	TArray<TObjectPtr<UR1ItemInstance>> GridData;

	UPROPERTY()
	TArray<TObjectPtr<UR1ItemInstance>> Items;

	// 현재 장착된 아이템들을 슬롯 부위별로 관리하는 맵

	UPROPERTY()
	TMap <ER1EquipmentSlot, TObjectPtr<UR1ItemInstance>> EquippedItems;

public:
	// 헤더 파일에 데이터 테이블 변수 추가
	UPROPERTY()
	TObjectPtr<UDataTable> ItemDataTable;

public:
	// 💡 아이템 크기에 맞는 빈 공간을 찾아 좌표를 반환해 주는 함수
	bool FindEmptySlot(const FIntPoint& ItemSize, FIntPoint& OutPos);

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UR1ItemInstance* GetEquippedItem(ER1EquipmentSlot SlotType) const
	{
		if (const TObjectPtr<UR1ItemInstance>* FoundItem = EquippedItems.Find(SlotType))
		{
			return *FoundItem;
		}
		return nullptr;
	}
};
