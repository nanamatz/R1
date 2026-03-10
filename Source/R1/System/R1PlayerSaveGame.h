

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
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
	ER1MinimapRoomState MinimapState;

	UPROPERTY()
	FIntPoint GridPosition;

	UPROPERTY()
	TArray<int32> ConnectedNodeIDs;

	UPROPERTY()
	FName RoomAssetName;
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
	float MoveSpeed = 0.f;

	// 맵 데이터
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentFloorIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentActiveNodeID = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<FR1MapNodeSaveData> SavedMapNodes;

};
