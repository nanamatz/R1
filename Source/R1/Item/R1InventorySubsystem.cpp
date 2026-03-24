


#include "Item/R1InventorySubsystem.h"
#include "Item/R1ItemInstance.h"
#include "System/R1EquipmentManagerComponent.h"
#include "Data/R1ItemAssetData.h"
#include "Object/R1ItemActor.h"
#include "Object/R1MerchantNPC.h"

void UR1InventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GridData.Init(nullptr, GetInventoryColumns() * GetInventoryRows());
}

void UR1InventorySubsystem::Deinitialize()
{
	Super::Deinitialize();
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

bool UR1InventorySubsystem::RemoveItem(UR1ItemInstance* ItemToRemove)
{
	if (!ItemToRemove) return false;

	// 1. 그리드에서 아이템의 위치를 찾습니다.
	FIntPoint ItemPos = GetItemPosition(ItemToRemove);

	// 2. 그리드 좌표가 유효하다면 그리드 점유를 해제합니다.
	if (ItemPos != FIntPoint(-1, -1))
	{
		RemoveItemFromGrid(ItemToRemove, ItemPos);
	}

	// 3. 인벤토리 목록 배열에서 아이템을 최종 삭제합니다.
	int32 RemovedCount = Items.Remove(ItemToRemove);

	// 삭제가 성공적으로 이루어졌다면 UI 갱신 방송
	if (RemovedCount > 0)
	{
		OnInventoryUpdated.Broadcast();
		return true;
	}

	return false;
}

UR1EquipmentManagerComponent* UR1InventorySubsystem::GetEquipmentManager() const
{
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APawn* PlayerPawn = PC->GetPawn())
			{
				return PlayerPawn->FindComponentByClass<UR1EquipmentManagerComponent>();
			}
		}
	}
	return nullptr;
}

UR1ItemInstance* UR1InventorySubsystem::CreateItemInstance(UR1ItemAssetData* InItemData, EItemRarity Rarity)
{
	if (!InItemData) return nullptr;
	UR1ItemInstance* NewItem = NewObject<UR1ItemInstance>(this);
	NewItem->Init(InItemData, Rarity);
	return NewItem;
}

bool UR1InventorySubsystem::EquipItem(UR1ItemInstance* ItemToEquip, ER1EquipmentSlot SpecificSlot)
{
	if (!ItemToEquip) return false;

	ER1EquipmentSlot TargetSlot = (SpecificSlot != ER1EquipmentSlot::None) ? SpecificSlot : ItemToEquip->GetEquipSlot()[0];
	if (TargetSlot == ER1EquipmentSlot::None) return false;

	// 💡 1. 현재 인벤토리 그리드에 있다면 제거
	FIntPoint CurrentPos = GetItemPosition(ItemToEquip);
	if (CurrentPos != FIntPoint(-1, -1))
	{
		RemoveItemFromGrid(ItemToEquip, CurrentPos);
	}

	// 💡 2. 해당 슬롯에 이미 아이템이 있다면 해제 (순서: 내가 먼저 빠지고 얘가 들어가야 빈자리 활용 가능)
	if (EquippedItems.Contains(TargetSlot))
	{
		UnequipItem(TargetSlot, CurrentPos); // 💡 스왑 시 내가 있던 자리를 선호
	}

	Items.Remove(ItemToEquip);
	EquippedItems.Add(TargetSlot, ItemToEquip);
	OnInventoryUpdated.Broadcast();

	// 🌟 도우미 함수 적용으로 확 줄어든 코드!
	if (UR1EquipmentManagerComponent* EquipComp = GetEquipmentManager())
	{
		EquipComp->EquipItem(TargetSlot, ItemToEquip->GetItemData());
	}

	return true;
}

bool UR1InventorySubsystem::UnequipItem(ER1EquipmentSlot TargetSlot, FIntPoint PreferredPos)
{
	if (!EquippedItems.Contains(TargetSlot)) return false;

	UR1ItemInstance* ItemToUnequip = EquippedItems[TargetSlot];
	EquippedItems.Remove(TargetSlot);
	Items.Add(ItemToUnequip);

	// 💡 1. 선호되는 좌표가 있고, 거기에 놓을 수 있다면 알박기!
	if (PreferredPos != FIntPoint(-1, -1) && CanAddItemAt(ItemToUnequip->GetItemSize(), PreferredPos))
	{
		AddItemToGrid(ItemToUnequip, PreferredPos);
	}
	else
	{
		// 💡 2. 그렇지 않다면 빈 공간을 찾아 자동으로 배치
		FIntPoint EmptyPos;
		if (FindEmptySlot(ItemToUnequip->GetItemSize(), EmptyPos))
		{
			AddItemToGrid(ItemToUnequip, EmptyPos);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("인벤토리에 빈 공간이 없어 장비를 바닥에 버리거나 보관할 수 없습니다. (현재는 그냥 Items에만 추가)"));
		}
	}

	OnInventoryUpdated.Broadcast();

	// 🌟 도우미 함수 적용
	if (UR1EquipmentManagerComponent* EquipComp = GetEquipmentManager())
	{
		EquipComp->UnEquipItem(TargetSlot);
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
	if (UR1EquipmentManagerComponent* EquipComp = GetEquipmentManager())
	{
		for (auto& Pair : EquippedItems)
		{
			EquipComp->UnEquipItem(Pair.Key);
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

bool UR1InventorySubsystem::AddItem(UR1ItemAssetData* InItemData, EItemRarity Rarity, int32 InCount)
{
	return AddItemAt(InItemData, Rarity, InCount, FIntPoint(-1, -1));
}

bool UR1InventorySubsystem::AddItemAt(UR1ItemAssetData* InItemData, EItemRarity Rarity, int32 InCount, FIntPoint PreferredPos)
{
	if (!InItemData || InCount <= 0) return false;

	bool bIsStackable = (InItemData->ItemType == ER1ItemType::Consumable) || (InItemData->ItemType == ER1ItemType::Key);
	int32 RemainingCount = InCount;

	if (bIsStackable)
	{
		int32 MaxStack = 999; // 최대 겹치기 개수

		for (UR1ItemInstance* ExistingItem : Items)
		{
			// 가방에서 나랑 완전히 똑같은 종류의 아이템을 찾았다면?
			if (ExistingItem && ExistingItem->GetItemData() == InItemData && ExistingItem->ItemRarity == Rarity)
			{
				// 그리고 그 칸이 아직 999개가 안 돼서 여유 공간이 있다면?
				if (ExistingItem->ItemCount < MaxStack)
				{
					// 남은 공간만큼 꽉꽉 채워 넣습니다.
					int32 SpaceLeft = MaxStack - ExistingItem->ItemCount;
					int32 AmountToAdd = FMath::Min(RemainingCount, SpaceLeft);

					ExistingItem->ItemCount += AmountToAdd;
					RemainingCount -= AmountToAdd;

					// 전부 다 겹쳐서 남은 아이템이 0개가 되었다면?
					if (RemainingCount <= 0)
					{
						OnInventoryUpdated.Broadcast();
						return true; // 🌟 새 칸을 차지할 필요 없이 여기서 즉시 종료!
					}
				}
			}
		}

		// 겹치기를 다 했는데도 개수가 남았다면 (예: 기존 슬롯이 999개라 꽉 참)
		// 남은 개수만큼 새로운 칸에 넣어야 하므로 Count를 남은 개수로 갱신합니다.
		InCount = RemainingCount;
	}

	UR1ItemInstance* NewItem = CreateItemInstance(InItemData, Rarity);
	if (!bIsStackable)
	{
		RemainingCount = 1; // 장비는 강제로 1개로 고정!
	}
	if (!NewItem) return false;

	NewItem->ItemCount = RemainingCount;

	// 사용자가 원하는 위치가 있고, 거기에 넣을 수 있다면 무조건 1순위로 넣는다!
	if (PreferredPos != FIntPoint(-1, -1) && CanAddItemAt(NewItem->GetItemSize(), PreferredPos))
	{
		Items.Add(NewItem);
		AddItemToGrid(NewItem, PreferredPos);
		OnInventoryUpdated.Broadcast();
		return true;
	}

	// 원하는 위치에 못 넣거나(막힘), 지정된 위치가 없다면 기존처럼 빈 공간을 찾아 넣는다.
	FIntPoint EmptyPos;
	if (FindEmptySlot(NewItem->GetItemSize(), EmptyPos))
	{
		Items.Add(NewItem);
		AddItemToGrid(NewItem, EmptyPos);
		OnInventoryUpdated.Broadcast();
		return true;
	}

	NewItem->MarkAsGarbage();
	return false;
}

bool UR1InventorySubsystem::BuyItem(UR1ItemInstance* ItemToBuy)
{
	return BuyItemAt(ItemToBuy, FIntPoint(-1, -1));
}

bool UR1InventorySubsystem::BuyItemAt(UR1ItemInstance* ItemToBuy, FIntPoint PreferredPos)
{
	if (!ItemToBuy || !ItemToBuy->GetItemData()) return false;

	int32 Price = ItemToBuy->GetItemData()->BaseValue * ItemToBuy->ItemCount;


	if (Gold < Price)
	{
		UE_LOG(LogTemp, Warning, TEXT("골드 부족: 필요 %d, 현재 %d"), Price, Gold);
		return false;
	}
	ConsumeGold(Price);

	if (CurrentMerchantNPC)
	{
		CurrentMerchantNPC->RemoveItemFromSale(ItemToBuy);
	}

	OnShopUpdated.Broadcast();

	if (AddItemAt(ItemToBuy->GetItemData(), ItemToBuy->ItemRarity, ItemToBuy->ItemCount, PreferredPos))
	{
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("인벤토리가 꽉 찼습니다! 아이템을 발밑에 떨어뜨립니다."));

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APawn* PlayerPawn = PC->GetPawn())
			{
				// 플레이어의 현재 위치를 가져옵니다.
				FVector SpawnLocation = PlayerPawn->GetActorLocation();

				// 발밑 주변에 겹치지 않게 무작위로 흩뿌려지도록 오프셋을 줍니다 (반경 100 범위)
				SpawnLocation.X += FMath::RandRange(-100.0f, 100.0f);
				SpawnLocation.Y += FMath::RandRange(-100.0f, 100.0f);
				SpawnLocation.Z += 50.0f; // 바닥에 파묻히지 않게 Z축 살짝 띄움

				FRotator SpawnRotation = FRotator::ZeroRotator;
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				UClass* ItemActorClass = StaticLoadClass(AR1ItemActor::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Interactable/BP_ItemActor.BP_ItemActor_C'"));
				if (ItemActorClass)
				{
					AR1ItemActor* DroppedItemActor = World->SpawnActor<AR1ItemActor>(ItemActorClass, SpawnLocation, SpawnRotation, SpawnParams);

					if (DroppedItemActor)
					{
						// 스폰된 아이템 액터에 데이터를 주입하여 시각화 및 루팅 가능 상태로 만듭니다.
						DroppedItemActor->InitItem(ItemToBuy->GetItemData(), ItemToBuy->ItemRarity, ItemToBuy->ItemCount);
					}
				}

			}
		}
	}
	return true;
}

void UR1InventorySubsystem::LoadItem(UR1ItemAssetData* InItemData, EItemRarity Rarity, FIntPoint Pos)
{
	// 🌟 도우미 함수 적용
	if (UR1ItemInstance* Item = CreateItemInstance(InItemData, Rarity))
	{
		Items.Add(Item);
		AddItemToGrid(Item, Pos);
	}
}

void UR1InventorySubsystem::LoadEquippedItem(UR1ItemAssetData* InItemData, EItemRarity Rarity, ER1EquipmentSlot Slot)
{
	// 🌟 도우미 함수 1, 2 동시 적용
	if (UR1ItemInstance* Item = CreateItemInstance(InItemData, Rarity))
	{
		EquippedItems.Add(Slot, Item);

		if (UR1EquipmentManagerComponent* EquipComp = GetEquipmentManager())
		{
			EquipComp->EquipItem(Slot, Item->GetItemData());
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

bool UR1InventorySubsystem::ConsumeKeyItem()
{
	for (UR1ItemInstance* Item : Items)
	{
		if (Item && Item->GetItemData())
		{
			// (팁: "Key"나 "열쇠"라는 이름이 들어간 아이템을 열쇠로 취급합니다. 데이터에 맞게 수정 가능)
			FString NameStr = Item->GetItemData()->ItemName.ToString();
			if (Item->GetItemData()->ItemType == ER1ItemType::Key) // 이름으로 찾는 게 아니라 다른 방법을 사용하고 싶음)
			{
				Item->ItemCount--; // 개수 1개 차감

				if (Item->ItemCount <= 0)
				{
					// 개수가 0이 되면 인벤토리에서 완전히 삭제
					FIntPoint ItemPos = GetItemPosition(Item);
					RemoveItemFromGrid(Item, ItemPos);
					Items.Remove(Item);
				}

				OnInventoryUpdated.Broadcast(); // UI 갱신
				return true; // 소모 성공
			}
		}
	}
	return false; // 인벤토리에 열쇠가 없음
}

void UR1InventorySubsystem::AddGold(int32 Amount)
{
	if (Amount <= 0) return;

	Gold += Amount;
	OnGoldChanged.Broadcast(Gold);
}

bool UR1InventorySubsystem::ConsumeGold(int32 Amount)
{
	if (Amount <= 0 || Gold < Amount) return false;

	Gold -= Amount;
	OnGoldChanged.Broadcast(Gold);
	return true;
}

void UR1InventorySubsystem::SellItem(UR1ItemInstance* Item, int32 Quantity)
{
	if (!Item || !Item->GetItemData()) return;

	int32 UnitValue = Item->GetItemData()->BaseValue;
	int32 SaleValue = FMath::Max(1, FMath::FloorToInt(UnitValue * 0.7f)) * Quantity;

	AddGold(SaleValue);

	// 아이템 개수 차감
	Item->ItemCount -= Quantity;

	if (Item->ItemCount <= 0)
	{
		// 인벤토리에서 완전히 제거
		FIntPoint ItemPos = GetItemPosition(Item);
		if (ItemPos != FIntPoint(-1, -1))
		{
			RemoveItemFromGrid(Item, ItemPos);
		}
		Items.Remove(Item);
	}

	OnInventoryUpdated.Broadcast();
}
