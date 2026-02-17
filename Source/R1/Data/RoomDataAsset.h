

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoomDataAsset.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ERoomType : uint8
 {
     Start,
     Normal,
     Boss
 };

UCLASS(BlueprintType)
class R1_API URoomDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
    // 이 방의 타입
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Data")
    ERoomType RoomType;

    // 불러올 레벨 인스턴스 (메모리 낭비를 막기 위해 Soft 참조 사용)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Data")
    TSoftObjectPtr<UWorld> RoomLevelInstance;
};
