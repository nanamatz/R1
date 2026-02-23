


#include "Item/R1InventorySubsystem.h"
#include "Item/R1ItemInstance.h"


void UR1InventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GridData.Init(nullptr, GetInventoryColumns() * GetInventoryRows());
}

void UR1InventorySubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UR1InventorySubsystem::AddDefaultItem()
{
	// 1. 테스트 무기 (크기 2x3, 부위: Weapon)
	TObjectPtr<UR1ItemInstance> TestWeapon = NewObject<UR1ItemInstance>(this);
	TestWeapon->Init(101, FIntPoint(2, 3), ER1EquipmentSlot::Weapon);
	Items.Add(TestWeapon);

	// 2. 테스트 투구 (크기 2x2, 부위: Helmet)
	TObjectPtr<UR1ItemInstance> TestHelmet = NewObject<UR1ItemInstance>(this);
	TestHelmet->Init(102, FIntPoint(2, 2), ER1EquipmentSlot::Helmet);
	Items.Add(TestHelmet);

	TObjectPtr<UR1ItemInstance> TestGlove = NewObject<UR1ItemInstance>(this);
	TestGlove->Init(102, FIntPoint(2, 2), ER1EquipmentSlot::Glove);
	Items.Add(TestGlove);

	// 3. 테스트 반지 (크기 1x1, 부위: Ring)
	TObjectPtr<UR1ItemInstance> TestRing = NewObject<UR1ItemInstance>(this);
	TestRing->Init(103, FIntPoint(1, 1), ER1EquipmentSlot::Ring);
	Items.Add(TestRing);

	// 4. 일반 잡템 (크기 1x1, 부위: None - 장착 불가 테스트용)
	TObjectPtr<UR1ItemInstance> TestPotion = NewObject<UR1ItemInstance>(this);
	TestPotion->Init(201, FIntPoint(1, 1), ER1EquipmentSlot::None);
	Items.Add(TestPotion);


	FIntPoint CurrentPos = FIntPoint(0, 0);

	for (UR1ItemInstance* Item : Items)
	{
		if (!Item) continue;

		// 빈 공간을 찾아 아이템을 GridData에 알박기 합니다 (데이터 세팅)
		if (CanAddItemAt(Item->ItemSize, CurrentPos))
		{
			AddItemToGrid(Item, CurrentPos);

			CurrentPos.X += Item->ItemSize.X; // 다음 아이템을 위해 X좌표 밀기 (임시 로직)
		}
	}

	OnInventoryUpdated.Broadcast();
}

bool UR1InventorySubsystem::CanAddItemAt(const FIntPoint& ItemSize, const FIntPoint& TargetPos, UR1ItemInstance* IgnoreItem)
{
	if (TargetPos.X < 0 || TargetPos.Y < 0) return false;

	for (int32 X = 0; X < ItemSize.X; ++X)
	{
		for (int32 Y = 0; Y < ItemSize.Y; ++Y)
		{
			int32 CheckX = TargetPos.X + X;
			int32 CheckY = TargetPos.Y + Y;

			// 1. 인벤토리 벽을 뚫고 나가는가?
			if (CheckX >= GetInventoryColumns() || CheckY >= GetInventoryRows())
			{
				return false;
			}

			// 2. 다른 아이템과 겹치는가?
			int32 GridIndex = CheckY * GetInventoryColumns() + CheckX;
			if (GridData[GridIndex] != nullptr && GridData[GridIndex] != IgnoreItem)
			{
				return false;
			}
		}
	}
	return true;
}

void UR1InventorySubsystem::AddItemToGrid(UR1ItemInstance* Item, const FIntPoint& Pos)
{
	if (!Item) return;
	for (int32 X = 0; X < Item->ItemSize.X; ++X)
	{
		for (int32 Y = 0; Y < Item->ItemSize.Y; ++Y)
		{
			int32 GridIndex = (Pos.Y + Y) * GetInventoryColumns() + (Pos.X + X);
			if (GridData.IsValidIndex(GridIndex))
			{
				GridData[GridIndex] = Item;
			}
		}
	}
}

void UR1InventorySubsystem::RemoveItemFromGrid(UR1ItemInstance* Item, const FIntPoint& Pos)
{
	if (!Item) return;
	for (int32 X = 0; X < Item->ItemSize.X; ++X)
	{
		for (int32 Y = 0; Y < Item->ItemSize.Y; ++Y)
		{
			int32 GridIndex = (Pos.Y + Y) * GetInventoryColumns() + (Pos.X + X);
			if (GridData.IsValidIndex(GridIndex) && GridData[GridIndex] == Item)
			{
				GridData[GridIndex] = nullptr;
			}
		}
	}
}

void UR1InventorySubsystem::MoveItemInGrid(UR1ItemInstance* Item, FIntPoint OldPos, FIntPoint NewPos)
{
	RemoveItemFromGrid(Item, OldPos);
	AddItemToGrid(Item, NewPos);
}

bool UR1InventorySubsystem::EquipItem(UR1ItemInstance* ItemToEquip)
{
	if (!ItemToEquip || ItemToEquip->EquipSlot == ER1EquipmentSlot::None) return false;

	ER1EquipmentSlot TargetSlot = ItemToEquip->EquipSlot;

	// 1. 만약 해당 부위에 이미 낀 장비가 있다면? -> 뺀다 (스왑 처리)
	if (EquippedItems.Contains(TargetSlot))
	{
		UnequipItem(TargetSlot);
	}

	// 2. 인벤토리 목록에서 아이템 제거
	Items.Remove(ItemToEquip);

	// 3. 장비창(Map)에 등록
	EquippedItems.Add(TargetSlot, ItemToEquip);

	// 4. UI 갱신 알림!
	OnInventoryUpdated.Broadcast();

	UE_LOG(LogTemp, Warning, TEXT("장비 장착 완료: 슬롯 %d"), (int32)TargetSlot);
	return true;
}

bool UR1InventorySubsystem::UnequipItem(ER1EquipmentSlot TargetSlot)
{
	if (!EquippedItems.Contains(TargetSlot)) return false;

	// 장착된 아이템을 빼서
	UR1ItemInstance* ItemToUnequip = EquippedItems[TargetSlot];
	EquippedItems.Remove(TargetSlot);

	// 인벤토리로 다시 넣음 (나중에는 빈 1칸, 2x3칸 등을 찾아서 넣는 로직이 들어가야 함)
	Items.Add(ItemToUnequip);

	OnInventoryUpdated.Broadcast();

	return true;
}
