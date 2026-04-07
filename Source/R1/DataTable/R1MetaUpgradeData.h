

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "R1MetaUpgradeData.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FR1MetaUpgradeData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 업그레이드할 스탯의 태그 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	FGameplayTag UpgradeTag;

	// UI에 표시될 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	FText UpgradeName;

	// 이 스킬을 찍기 위해 필요한 플레이어의 최소 레벨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	int32 RequiredPlayerLevel = 1;

	// 최대 마스터 레벨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	int32 MaxLevel = 1;

	//  UI에 표시될 상세 설명 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta|UI", meta = (MultiLine = true))
	FText UpgradeDescription;

	// UI에 표시될 스킬 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta|UI")
	TObjectPtr<class UTexture2D> UpgradeIcon;

	// 각 레벨별 오르는 수치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	TArray<float> ValuesPerLevel;
};