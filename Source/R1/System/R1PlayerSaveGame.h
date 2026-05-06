

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "R1Define.h"
#include "R1PlayerSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FR1MapNodeSaveData
{
	GENERATED_BODY()

	UPROPERTY() 
	int32 NodeID;

	UPROPERTY() 
	bool bIsCleared;

	UPROPERTY()
	bool bIsVisited;

	UPROPERTY() 
	ER1MinimapRoomState MinimapState;

	UPROPERTY()
	FIntPoint GridPosition;

	UPROPERTY()
	TArray<int32> ConnectedNodeIDs;

	UPROPERTY()
	FName RoomAssetName;
};

USTRUCT(BlueprintType)
struct FR1ItemSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UR1ItemAssetData> ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemRarity ItemRarity = EItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Position = FIntPoint(-1, -1);
};

USTRUCT(BlueprintType)
struct FR1EquippedItemSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UR1ItemAssetData> ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemRarity ItemRarity = EItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ER1EquipmentSlot Slot = ER1EquipmentSlot::None;
};

USTRUCT(BlueprintType)
struct FR1ShopInventorySaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FR1ItemSaveData> Items;
};

/**
 * 
 */
UCLASS()
class R1_API UR1PlayerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 플레이어 스탯 데이터
	UPROPERTY(BlueprintReadWrite)
	float MaxHealth = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float Health = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float MaxMana = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float Mana = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float BaseDamage = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float BaseDefence = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float Exp = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float MaxExp = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float Level = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float AttackSpeed = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float AttackRange = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float MoveSpeed = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float CritChance = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float CritMultiplier = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float DamageMultiplier = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float HealthRegen = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float ManaRegen = 0.f;

	UPROPERTY(BlueprintReadWrite)
	int32 LastSettledLevel = 1;

	// [Added] 런 진행도 저장 데이터
	UPROPERTY(BlueprintReadWrite)
	int32 RunLevel = 1;

	UPROPERTY(BlueprintReadWrite)
	int32 AvailableRunUpgradePoints = 0;

	UPROPERTY(BlueprintReadWrite)
	TMap<FGameplayTag, int32> RunUpgradeHistory;

	// [Added] 플레이어 위치 및 회전 데이터
	UPROPERTY(BlueprintReadWrite)
	FVector PlayerLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FRotator PlayerRotation = FRotator::ZeroRotator;

	// 맵 데이터
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentFloorIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentActiveNodeID = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<FR1MapNodeSaveData> SavedMapNodes;

	// 인벤토리 아이템 데이터
	UPROPERTY(BlueprintReadWrite)
	TArray<FR1ItemSaveData> InventoryItems;

	// 장착된 아이템 데이터
	UPROPERTY(BlueprintReadWrite)
	TArray<FR1EquippedItemSaveData> EquippedItems;

	UPROPERTY(BlueprintReadWrite)
	TMap<int32, FR1ShopInventorySaveData> ShopInventories;
};
