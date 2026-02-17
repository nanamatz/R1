#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "R1RoomDefinitionData.generated.h"

UENUM(BlueprintType)
enum class ER1RoomContentType : uint8
{
	Start,
	Combat,
	Event,
	Boss,
};

/**
 * 룸 단위 스트리밍/선로딩에 사용하는 PrimaryDataAsset.
 * 기존 AssetData/PrimaryDataAsset 파이프라인에 맞춰 soft reference 중심으로 구성한다.
 */
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room|Preload")
	TArray<FPrimaryAssetId> PreloadPrimaryAssets;

	// 기존 AssetData 라벨 체계를 사용할 수 있도록 보조 라벨도 유지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room|Preload")
	TArray<FName> PreloadAssetLabels;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room|Spawn")
	int32 SpawnBatchSizePerFrame = 6;
};
