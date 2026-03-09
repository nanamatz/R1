

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "R1PlayerSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FR1MapNodeSaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 NodeID = -1;

	// 이 방이 어떤 라벨(종류)의 방이었는지 (예: Boss Map Lists)
	UPROPERTY(BlueprintReadWrite)
	FName RoomLabel;

	// 이 방을 클리어했는지 여부 (이게 있어야 불러왔을 때 몬스터를 스폰할지 말지 결정합니다)
	UPROPERTY(BlueprintReadWrite)
	bool bIsCleared = false;

	// 그리드 상의 위치
	UPROPERTY(BlueprintReadWrite)
	FIntPoint GridPosition = FIntPoint::ZeroValue;

	// 연결된 문(노드) 정보
	UPROPERTY(BlueprintReadWrite)
	TArray<int32> ConnectedNodeIDs;
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
