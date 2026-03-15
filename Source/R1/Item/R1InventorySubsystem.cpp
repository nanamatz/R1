


#include "Item/R1InventorySubsystem.h"
#include "Item/R1ItemInstance.h"
#include "System/R1EquipmentManagerComponent.h"


void UR1InventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GridData.Init(nullptr, GetInventoryColumns() * GetInventoryRows());

	// 💡 서브시스템 시작할 때 딱 한 번 로드!
	// (경로는 본인의 프로젝트 경로로 꼭 수정하세요!)
	FString Path = TEXT("/Script/Engine.DataTable'/Game/DataTable/DT_ItemDataTable.DT_ItemDataTable'");
	ItemDataTable = LoadObject<UDataTable>(nullptr, *Path);

	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("서브시스템 초기화 실패: 데이터 테이블을 못 찾았습니다!"));
	}
}

void UR1InventorySubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UR1InventorySubsystem::AddDefaultItem()
{
	if (!ItemDataTable) return;

	TObjectPtr<UR1ItemInstance> TestHelemt = NewObject<UR1ItemInstance>(this);
	TestHelemt->Init(1, ItemDataTable);
	Items.Add(TestHelemt);

	TObjectPtr<UR1ItemInstance> TestWeapon = NewObject<UR1ItemInstance>(this);
	TestWeapon->Init(2, ItemDataTable);
	Items.Add(TestWeapon);

	TObjectPtr<UR1ItemInstance> TestArmor = NewObject<UR1ItemInstance>(this);
	TestArmor->Init(3, ItemDataTable);
	Items.Add(TestArmor);

	TObjectPtr<UR1ItemInstance> TestGlove = NewObject<UR1ItemInstance>(this);
	TestGlove->Init(4, ItemDataTable);
	Items.Add(TestGlove);

	TObjectPtr<UR1ItemInstance> TestBoots = NewObject<UR1ItemInstance>(this);
	TestBoots->Init(5, ItemDataTable);
	Items.Add(TestBoots);

	TObjectPtr<UR1ItemInstance> TestRing = NewObject<UR1ItemInstance>(this);
	TestRing->Init(6, ItemDataTable);
	Items.Add(TestRing);

	TObjectPtr<UR1ItemInstance> TestRing2 = NewObject<UR1ItemInstance>(this);
	TestRing2->Init(6, ItemDataTable);
	Items.Add(TestRing2);

	TObjectPtr<UR1ItemInstance> TestPotion = NewObject<UR1ItemInstance>(this);
	TestPotion->Init(7, ItemDataTable);
	Items.Add(TestPotion);

	for (UR1ItemInstance* Item : Items)
	{
		if (!Item) continue;

		FIntPoint EmptyPos;

		// 💡 1. 우리가 만든 도우미 함수에게 "이 아이템 크기 들어갈 빈자리 좀 찾아와!" 라고 시킵니다.
		if (FindEmptySlot(Item->GetItemSize(), EmptyPos))
		{
			// 2. 빈자리를 찾았다면 그 위치에 알박기!
			AddItemToGrid(Item, EmptyPos);
		}
		else
		{
			// 3. 만약 Y축 끝까지 다 뒤졌는데도 자리가 없다면? (인벤토리 풀 상태)
			UE_LOG(LogTemp, Warning, TEXT("인벤토리가 꽉 차서 임시 아이템(ID: %d)을 배치하지 못했습니다!"), Item->ItemID);
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
	for (int32 X = 0; X < Item->GetItemSize().X; ++X)
	{
		for (int32 Y = 0; Y < Item->GetItemSize().Y; ++Y)
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
	for (int32 X = 0; X < Item->GetItemSize().X; ++X)
	{
		for (int32 Y = 0; Y < Item->GetItemSize().Y; ++Y)
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

bool UR1InventorySubsystem::EquipItem(UR1ItemInstance* ItemToEquip, ER1EquipmentSlot SpecificSlot)
{
	if (!ItemToEquip) return false;

	ER1EquipmentSlot TargetSlot = (SpecificSlot != ER1EquipmentSlot::None) ? SpecificSlot : ItemToEquip->GetEquipSlot()[0];

	if (TargetSlot == ER1EquipmentSlot::None) return false;

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

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			// 플레이어 몸에 붙어있는 장비 관리자 컴포넌트를 찾습니다.
			if (UR1EquipmentManagerComponent* EquipComp = PlayerPawn->FindComponentByClass<UR1EquipmentManagerComponent>())
			{
				// 아이템 인스턴스 안에 있는 데이터 행(Row)을 컴포넌트에게 넘겨줍니다!
				EquipComp->EquipItem(TargetSlot, ItemToEquip->GetItemData());
			}
		}
	}

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

	// 💡 만약 이 아이템이 그리드(GridData) 어디에도 속해있지 않다면, 빈자리를 찾아 넣어줍니다.
	// (단순 변심으로 인한 해제나, 스왑 시 위치를 지정받지 못한 경우 대비)
	if (GetItemPosition(ItemToUnequip) == FIntPoint(-1, -1))
	{
		FIntPoint EmptyPos;
		if (FindEmptySlot(ItemToUnequip->GetItemSize(), EmptyPos))
		{
			AddItemToGrid(ItemToUnequip, EmptyPos);
		}
	}

	OnInventoryUpdated.Broadcast();

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			if (UR1EquipmentManagerComponent* EquipComp = PlayerPawn->FindComponentByClass<UR1EquipmentManagerComponent>())
			{
				EquipComp->UnEquipItem(TargetSlot);
			}
		}
	}

	return true;
}

FIntPoint UR1InventorySubsystem::GetItemPosition(UR1ItemInstance* Item) const
{
	if (!Item) return FIntPoint(-1, -1);
	for (int32 Y = 0; Y < GetInventoryRows(); ++Y)
	{
		for (int32 X = 0; X < GetInventoryColumns(); ++X)
		{
			if (GridData[Y * GetInventoryColumns() + X] == Item)
			{
				return FIntPoint(X, Y);
			}
		}
	}
	return FIntPoint(-1, -1);
}

void UR1InventorySubsystem::ClearInventory()
{
	// 1. 장착된 장비 해제 (단순히 데이터만 지움, GAS 등은 밖에서 처리하거나 여기서 EquipComp를 가져와서 해제해야 함)
	// 하지만 세이브 로드 시에는 보통 모든 장비를 해제하고 새로 장착하는 게 깔끔함.
	
	// EquippedItems 맵 순회하면서 GAS 해제도 필요할 수 있음
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			if (UR1EquipmentManagerComponent* EquipComp = PlayerPawn->FindComponentByClass<UR1EquipmentManagerComponent>())
			{
				for (auto& Pair : EquippedItems)
				{
					EquipComp->UnEquipItem(Pair.Key);
				}
			}
		}
	}

	EquippedItems.Empty();
	Items.Empty();
	for (int32 i = 0; i < GridData.Num(); ++i)
	{
		GridData[i] = nullptr;
	}

	OnInventoryUpdated.Broadcast();
}

void UR1InventorySubsystem::LoadItem(int32 ItemID, EItemRarity Rarity, FIntPoint Pos)
{
	if (!ItemDataTable) return;

	TObjectPtr<UR1ItemInstance> Item = NewObject<UR1ItemInstance>(this);
	Item->Init(ItemID, ItemDataTable);
	Item->ItemRarity = Rarity;

	Items.Add(Item);
	AddItemToGrid(Item, Pos);
}

void UR1InventorySubsystem::LoadEquippedItem(int32 ItemID, EItemRarity Rarity, ER1EquipmentSlot Slot)
{
	if (!ItemDataTable) return;

	TObjectPtr<UR1ItemInstance> Item = NewObject<UR1ItemInstance>(this);
	Item->Init(ItemID, ItemDataTable);
	Item->ItemRarity = Rarity;

	EquippedItems.Add(Slot, Item);

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			if (UR1EquipmentManagerComponent* EquipComp = PlayerPawn->FindComponentByClass<UR1EquipmentManagerComponent>())
			{
				EquipComp->EquipItem(Slot, Item->GetItemData());
			}
		}
	}
}

bool UR1InventorySubsystem::FindEmptySlot(const FIntPoint& ItemSize, FIntPoint& OutPos)
{
	// (0, 0)부터 인벤토리 끝까지 싹 스캔합니다.
	for (int32 Y = 0; Y < GetInventoryRows(); ++Y)
	{
		for (int32 X = 0; X < GetInventoryColumns(); ++X)
		{
			FIntPoint CheckPos(X, Y);
			// 내가 만든 겹침 검사 함수(CanAddItemAt) 재활용!
			if (CanAddItemAt(ItemSize, CheckPos))
			{
				OutPos = CheckPos; // 빈자리 발견!
				return true;
			}
		}
	}
	// 빈자리가 없음 (인벤토리 꽉 참)
	UE_LOG(LogTemp, Error, TEXT("인벤토리 꽉 찼음"));
	return false; 
}
