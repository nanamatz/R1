#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Map/R1Door.h"
#include "Data/R1ItemPoolData.h"
#include "R1RoomDefinitionData.generated.h"

UENUM(BlueprintType)
enum class ER1RoomContentType : uint8
{
	None,
	Start,
	Combat,
	Event,
	Boss,
	Treasure,
	Shop,
	Refresh
};

UCLASS(BlueprintType)
class R1_API UR1RoomDefinitionData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	ER1RoomContentType RoomType = ER1RoomContentType::Combat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	TSoftObjectPtr<UWorld> RoomLevel;

	// 룸 진입 전에 함께 로드할 Primary Assets (몬스터 아키타입, 드랍, 테마 VFX/SFX 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	TArray<FPrimaryAssetId> PreloadPrimaryAssets;

	// 기존 AssetData 라벨 체계를 사용할 수 있도록 보조 라벨도 유지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	TArray<FName> PreloadAssetLabels;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	int32 SpawnBatchSizePerFrame = 6;

	// 방이 물리적으로 가지고 있는 문의 방향들 (에디터에서 체크해 둡니다)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Settings")
	TArray<ER1DoorDirection> AvailableDoors;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room|Loot")
	TObjectPtr<UR1ItemPoolData> RoomClearLootPool;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room|Loot", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float RoomClearDropChance = 30.0f;
};
