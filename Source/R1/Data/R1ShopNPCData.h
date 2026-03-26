

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
	// 상인 이름 (예: "멍멍Dog")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Info")
	FText NPCName;

	// 상인 초상화 이미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Info")
	TObjectPtr<UTexture2D> NPCPortrait;

	 UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Info")
	 TArray<FText> GreetingDialogues;
};
