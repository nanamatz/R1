


#include "R1SaveSystem.h"
#include "Kismet/GameplayStatics.h"
#include "R1PlayerSaveGame.h" // 우리가 수정한 세이브 객체 헤더

#include "Character/R1Player.h"

#include "Player/R1PlayerState.h"

#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"

#include "Map/R1MapGenerator.h"
#include "Data/R1RoomDefinitionData.h"
#include "System/R1MetaSaveGame.h"
#include "Item/R1InventorySubsystem.h"
#include "Item/R1ItemInstance.h"



bool UR1SaveSystem::HasSavedRun() const
{
	return UGameplayStatics::DoesSaveGameExist(RunSaveSlotName, RunSaveUserIndex);
}

void UR1SaveSystem::DeleteSavedRun()
{
	ExtractAndSaveMetaProgression();

	if (HasSavedRun())
	{
		UGameplayStatics::DeleteGameInSlot(RunSaveSlotName, RunSaveUserIndex);
	}

	// 세이브 파일뿐만 아니라 현재 인메모리 상의 인벤토리 상태도 초기화
	if (UWorld* World = GetWorld())
	{
		if (UR1InventorySubsystem* InventorySubsystem = World->GetSubsystem<UR1InventorySubsystem>())
		{
			InventorySubsystem->ClearInventory();
		}
	}
	ActiveShopInventories.Empty();
}

void UR1SaveSystem::SaveCurrentRun(AR1Player* Player, AR1MapGenerator* MapGenerator)
{
	UR1PlayerSaveGame* SaveObj = Cast<UR1PlayerSaveGame>(UGameplayStatics::CreateSaveGameObject(UR1PlayerSaveGame::StaticClass()));
	if (!SaveObj) return;

	// 1. 플레이어 상태 저장 (작성하셨던 코드 재활용!)
	if (Player)
	{
		// [Added] 위치 및 회전 저장
		SaveObj->PlayerLocation = Player->GetActorLocation();
		SaveObj->PlayerRotation = Player->GetActorRotation();

		if (AR1PlayerState* PS = Cast<AR1PlayerState>(Player->GetPlayerState()))
		{
			if (UR1AttributeSet* CommonAttr = Cast<UR1AttributeSet>(PS->GetCommonAttributeSet()))
			{
				SaveObj->MaxHealth = CommonAttr->GetMaxHealth();
				SaveObj->Health = CommonAttr->GetHealth();

				SaveObj->BaseDamage = CommonAttr->GetBaseDamage();
				SaveObj->BaseDefence = CommonAttr->GetBaseDefence();
				SaveObj->MoveSpeed = CommonAttr->GetMoveSpeed();
				SaveObj->AttackSpeed = CommonAttr->GetAttackSpeed();
			}
			if (UPlayerAttributeSet* PlayerAttr = Cast<UPlayerAttributeSet>(PS->GetPlayerAttributeSet()))
			{
				SaveObj->MaxMana = PlayerAttr->GetMaxMana();
				SaveObj->Mana = PlayerAttr->GetMana();
				SaveObj->Level = PlayerAttr->GetLevel();
				SaveObj->Exp = PlayerAttr->GetExp();
				SaveObj->MaxExp = PlayerAttr->GetMaxExp();
			}
		}
	}

	// 2. 맵 데이터 압축 및 저장
	if (MapGenerator)
	{
		SaveObj->CurrentFloorIndex = MapGenerator->CurrentFloorIndex;
		SaveObj->CurrentActiveNodeID = MapGenerator->CurrentActiveNodeID;
		SaveObj->CurrentActiveNodeID = (MapGenerator->PendingNodeID != -1) ? MapGenerator->PendingNodeID : MapGenerator->CurrentActiveNodeID;

		// MapGenerator가 들고 있는 무거운 GeneratedMap을 가벼운 SaveNode로 변환
		for (const FR1MapNode& Node : MapGenerator->GeneratedMap)
		{
			FR1MapNodeSaveData SaveNode;
			SaveNode.NodeID = Node.NodeID;
			SaveNode.MinimapState = Node.MinimapState;
			SaveNode.bIsCleared = Node.bIsCleared;
			SaveNode.bIsVisited = Node.bIsVisited;
			SaveNode.GridPosition = Node.GridPosition;
			SaveNode.ConnectedNodeIDs = Node.ConnectedNodeIDs;

			if (Node.RoomDefinition)
			{
				SaveNode.RoomAssetName = Node.RoomDefinition->GetFName();
			}
			else
			{
				SaveNode.RoomAssetName = NAME_None;
			}

			SaveObj->SavedMapNodes.Add(SaveNode);
		}
	}

	// 3. 인벤토리 및 장착 아이템 저장
	if (UWorld* World = GetWorld())
	{
		if (UR1InventorySubsystem* InventorySubsystem = World->GetSubsystem<UR1InventorySubsystem>())
		{
			// 인벤토리 아이템 저장
			for (UR1ItemInstance* Item : InventorySubsystem->GetItems())
			{
				if (Item && Item->GetItemData())
				{
					FR1ItemSaveData ItemSaveData;
					ItemSaveData.ItemData = Item->GetItemData();
					ItemSaveData.ItemRarity = Item->ItemRarity;
					ItemSaveData.Position = InventorySubsystem->GetItemPosition(Item);
					SaveObj->InventoryItems.Add(ItemSaveData);
				}
			}

			// 장착된 아이템 저장
			for (auto& Pair : InventorySubsystem->GetEquippedItems())
			{
				if (Pair.Value && Pair.Value->GetItemData())
				{
					FR1EquippedItemSaveData EquippedSaveData;
					EquippedSaveData.ItemData = Pair.Value->GetItemData();
					EquippedSaveData.ItemRarity = Pair.Value->ItemRarity;
					EquippedSaveData.Slot = Pair.Key;
					SaveObj->EquippedItems.Add(EquippedSaveData);
				}
			}
		}
	}
	
	SaveObj->ShopInventories = ActiveShopInventories;

	UGameplayStatics::SaveGameToSlot(SaveObj, RunSaveSlotName, RunSaveUserIndex);
}

bool UR1SaveSystem::LoadCurrentRun(AR1Player* Player, AR1MapGenerator* MapGenerator)
{
	if (!HasSavedRun()) return false;

	UR1PlayerSaveGame* SaveObj = Cast<UR1PlayerSaveGame>(UGameplayStatics::LoadGameFromSlot(RunSaveSlotName, RunSaveUserIndex));
	if (!SaveObj) return false;

	// 1. 플레이어 상태 주입
	if (Player)
	{
		if (AR1PlayerState* PS = Cast<AR1PlayerState>(Player->GetPlayerState()))
		{
			if (UR1AttributeSet* CommonAttr = Cast<UR1AttributeSet>(PS->GetCommonAttributeSet()))
			{
				CommonAttr->SetMaxHealth(SaveObj->MaxHealth);
				CommonAttr->SetHealth(SaveObj->Health);
				CommonAttr->SetBaseDamage(SaveObj->BaseDamage);
				CommonAttr->SetBaseDefence(SaveObj->BaseDefence);
				CommonAttr->SetMoveSpeed(SaveObj->MoveSpeed);
				CommonAttr->SetAttackSpeed(SaveObj->AttackSpeed);
			}
			if (UPlayerAttributeSet* PlayerAttr = Cast<UPlayerAttributeSet>(PS->GetPlayerAttributeSet()))
			{
				PlayerAttr->SetLevel(SaveObj->Level);
				PlayerAttr->SetMaxExp(SaveObj->MaxExp);
				PlayerAttr->SetExp(SaveObj->Exp);
				PlayerAttr->SetMaxMana(SaveObj->MaxMana);
				PlayerAttr->SetMana(SaveObj->Mana);
			}
		}
	}

	if (MapGenerator)
	{
		// [Modified] 위치와 회전값도 함께 넘겨줌
		MapGenerator->LoadMapFromSaveData(SaveObj->SavedMapNodes, SaveObj->CurrentFloorIndex, SaveObj->CurrentActiveNodeID, SaveObj->PlayerLocation, SaveObj->PlayerRotation);
	}

	// 3. 인벤토리 및 장착 아이템 복구
	if (UWorld* World = GetWorld())
	{
		if (UR1InventorySubsystem* InventorySubsystem = World->GetSubsystem<UR1InventorySubsystem>())
		{
			InventorySubsystem->ClearInventory();

			// 인벤토리 아이템 복구
			for (const FR1ItemSaveData& ItemSaveData : SaveObj->InventoryItems)
			{
				if (UR1ItemAssetData* ResolvedData = ItemSaveData.ItemData.LoadSynchronous())
				{
					InventorySubsystem->LoadItem(ResolvedData, ItemSaveData.ItemRarity, ItemSaveData.Position);
				}
			}

			// 장착 아이템 복구
			for (const FR1EquippedItemSaveData& EquippedSaveData : SaveObj->EquippedItems)
			{
				if (UR1ItemAssetData* ResolvedData = EquippedSaveData.ItemData.LoadSynchronous())
				{
					InventorySubsystem->LoadEquippedItem(ResolvedData, EquippedSaveData.ItemRarity, EquippedSaveData.Slot);
				}
			}

			// 💡 로딩이 끝났음을 UI에 알려서 인벤토리/장비창을 갱신하게 만듭니다!
			InventorySubsystem->OnInventoryUpdated.Broadcast();
		}
	}

	ActiveShopInventories = SaveObj->ShopInventories;

	return true;
}

void UR1SaveSystem::SaveMetaProgression(UR1MetaSaveGame* MetaSaveObj)
{
	if (MetaSaveObj)
	{
		UGameplayStatics::SaveGameToSlot(MetaSaveObj, MetaSaveSlotName, MetaSaveUserIndex);
	}
}

UR1MetaSaveGame* UR1SaveSystem::LoadMetaProgression()
{
	if (UGameplayStatics::DoesSaveGameExist(MetaSaveSlotName, MetaSaveUserIndex))
	{
		return Cast<UR1MetaSaveGame>(UGameplayStatics::LoadGameFromSlot(MetaSaveSlotName, MetaSaveUserIndex));
	}

	UR1MetaSaveGame* NewMetaSave = Cast<UR1MetaSaveGame>(UGameplayStatics::CreateSaveGameObject(UR1MetaSaveGame::StaticClass()));
	NewMetaSave->PlayerMetaLevel = 1;
	NewMetaSave->AvailableSkillPoints = 1;
	return NewMetaSave;
}

void UR1SaveSystem::SaveShopInventory(int32 RoomID, const TArray<class UR1ItemInstance*>& ShopItems)
{
	FR1ShopInventorySaveData SaveData;
	for (UR1ItemInstance* Item : ShopItems)
	{
		if (Item && Item->GetItemData())
		{
			FR1ItemSaveData ItemSaveData;
			ItemSaveData.ItemData = Item->GetItemData();
			ItemSaveData.ItemRarity = Item->ItemRarity;
			SaveData.Items.Add(ItemSaveData);
		}
	}
	// 방 번호를 키값으로 캐시 갱신
	ActiveShopInventories.Add(RoomID, SaveData);
}

bool UR1SaveSystem::LoadShopInventory(int32 RoomID, TArray<class UR1ItemInstance*>& OutShopItems, UObject* Outer)
{
	if (!ActiveShopInventories.Contains(RoomID)) return false;

	OutShopItems.Empty();
	for (const FR1ItemSaveData& ItemData : ActiveShopInventories[RoomID].Items)
	{
		if (UR1ItemAssetData* ResolvedData = ItemData.ItemData.LoadSynchronous())
		{
			UR1ItemInstance* NewItem = NewObject<UR1ItemInstance>(Outer);
			NewItem->Init(ResolvedData, ItemData.ItemRarity);
			NewItem->ItemCount = 1;
			OutShopItems.Add(NewItem);
		}
	}
	return true;
}

void UR1SaveSystem::ExtractAndSaveMetaProgression()
{
	if (!HasSavedRun()) return;

	UR1PlayerSaveGame* RunSave = Cast<UR1PlayerSaveGame>(UGameplayStatics::LoadGameFromSlot(RunSaveSlotName, RunSaveUserIndex));
	if (!RunSave) return;

	UR1MetaSaveGame* MetaSave = LoadMetaProgression();
	if (!MetaSave) return;

	int32 EarnedPoints = FMath::Max(0, FMath::FloorToInt(RunSave->Level) - MetaSave->PlayerMetaLevel);

	if (EarnedPoints > 0)
	{
		MetaSave->AvailableSkillPoints += EarnedPoints;

		MetaSave->PlayerMetaLevel = FMath::FloorToInt(RunSave->Level);
	}

	MetaSave->CurrentMetaExp = RunSave->Exp;

	SaveMetaProgression(MetaSave);
}