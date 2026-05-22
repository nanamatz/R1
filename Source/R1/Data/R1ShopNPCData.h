

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "R1ShopNPCData.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1ShopNPCData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Info")
	FText NPCName;

	// DT_Localization 조회 키 (비어 있으면 NPCName 사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Info")
	FName NPCNameKey;

	// 상인 초상화 이미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Info")
	TObjectPtr<UTexture2D> NPCPortrait;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Info")
	TArray<FText> GreetingDialogues;

	// GreetingDialogues 각 항목에 대응하는 DT_Localization 키 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Info")
	TArray<FName> GreetingKeys;
};
