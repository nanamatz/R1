

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "R1Define.h"
#include "R1ItemFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1ItemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// 🌟 어디서든(C++, BP) 접근 가능한 스태틱(Static) 함수들
	UFUNCTION(BlueprintPure, Category = "UI")
	static FSlateColor GetRarityColor(EItemRarity Rarity);

	UFUNCTION(BlueprintPure, Category = "UI", meta = (WorldContext = "WorldContextObject"))
	static FText GetRarityText(UObject* WorldContextObject, EItemRarity Rarity);

	UFUNCTION(BlueprintPure, Category = "UI", meta = (WorldContext = "WorldContextObject"))
	static FText GetItemTypeText(UObject* WorldContextObject, ER1ItemType ItemType);

	UFUNCTION(BlueprintPure, Category = "UI", meta = (WorldContext = "WorldContextObject"))
	static FText GetEquipSlotText(UObject* WorldContextObject, ER1EquipmentSlot EquipSlot);

	static FName GetStatLocalizationKey(const FGameplayTag& Tag);
};
