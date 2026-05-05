

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "R1MetaSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1MetaSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	// 플레이어의 누적 (메타) 레벨
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 PlayerMetaLevel = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 CurrentMetaExp = 0;

	// 현재 로비에서 사용 가능한 스킬 포인트
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 AvailableSkillPoints = 0;

	// 영구 투자된 스탯 정보 (Key: 업그레이드 태그, Value: 찍은 레벨)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, int32> InvestedUpgrades;
};
