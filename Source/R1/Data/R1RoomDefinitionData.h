#pragma once

/**
 * [파일 역할]
 * 룸(Room) 단위의 스트리밍 및 선로딩(Preload)을 위한 핵심 데이터를 정의하는 Primary Data Asset 클래스입니다.
 * 룸의 유형(전투, 보스 등), 해당 룸에서 사용하는 레벨(Map), 그리고 룸 진입 전에 미리 로드해야 할 
 * 몬스터, 아이템, 이펙트 등의 에셋 목록을 관리합니다.
 * 
 * [필요성]
 * 1. 데이터 기반 설계: 하드코딩 없이 데이터 에셋만으로 룸의 구성 요소와 의존성을 정의할 수 있습니다.
 * 2. 효율적인 리소스 관리: Soft Object Pointer와 PrimaryAssetId를 사용하여, 필요한 시점에만 
 *    자원을 로드할 수 있는 구조를 제공하여 메모리 사용량을 최적화합니다.
 */

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
