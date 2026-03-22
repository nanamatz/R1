

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/R1ItemAssetData.h"
#include "R1InventorySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32, NewGold);
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

	const TArray<TObjectPtr<UR1ItemInstance>>& GetItems() { return Items; }

	const TMap<ER1EquipmentSlot, TObjectPtr<UR1ItemInstance>>& GetEquippedItems() const { return EquippedItems; }

	FIntPoint GetItemPosition(UR1ItemInstance* Item) const;

	void ClearInventory();

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
	// 인벤토리에서 특정 아이템 인스턴스를 완전히 제거하는 범용 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(UR1ItemInstance* ItemToRemove);

protected:
	// 🌟 중복 제거용 도우미 함수 1: 장비 매니저 컴포넌트 가져오기
	class UR1EquipmentManagerComponent* GetEquipmentManager() const;

	// 🌟 중복 제거용 도우미 함수 2: 아이템 인스턴스 생성하기
	class UR1ItemInstance* CreateItemInstance(class UR1ItemAssetData* InItemData, EItemRarity Rarity);

public:
	// 아이템을 인벤토리에서 장비창으로 이동
	bool EquipItem(UR1ItemInstance* ItemToEquip, ER1EquipmentSlot SpecificSlot = ER1EquipmentSlot::None);

	// 장비를 해제하여 다시 인벤토리로 이동
	bool UnequipItem(ER1EquipmentSlot TargetSlot, FIntPoint PreferredPos = FIntPoint(-1, -1));

	UPROPERTY(BlueprintAssignable)
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnGoldChanged OnGoldChanged;

public:
	// 골드 관리 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory|Gold")
	int32 GetGold() const { return Gold; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Gold")
	void AddGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Gold")
	bool ConsumeGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SellItem(UR1ItemInstance* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool BuyItem(class UR1ItemAssetData* InItemData, EItemRarity Rarity = EItemRarity::Common, int32 Count = 1);

public:
	UPROPERTY()
	int32 Gold = 0;

public:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Shop")
	bool bIsShopOpen = false;

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
	// 🌟 외부에서(예: 루팅할 때) 쉽게 아이템을 넣을 수 있는 만능 함수 추가
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(class UR1ItemAssetData* InItemData, EItemRarity Rarity = EItemRarity::Common, int32 Count = 1);

	// 🌟 세이브/로드 시 숫자 ID 대신 데이터 에셋 포인터를 받도록 수정
	void LoadItem(class UR1ItemAssetData* InItemData, EItemRarity Rarity, FIntPoint Pos);
	void LoadEquippedItem(class UR1ItemAssetData* InItemData, EItemRarity Rarity, ER1EquipmentSlot Slot);

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

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool ConsumeKeyItem();
};
