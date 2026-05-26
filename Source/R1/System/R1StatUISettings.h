

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "R1StatUISettings.generated.h"

class UTexture2D;


USTRUCT(BlueprintType)
struct FR1StatUIInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UTexture2D> StatIcon;
};
/**
 * 
 */
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "R1 스탯 UI 설정"))
class R1_API UR1StatUISettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	// 이 TMap 하나로 게임 내의 모든 스탯 아이콘과 이름을 매핑합니다.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Stats")
	TMap<FGameplayTag, FR1StatUIInfo> StatUIMap;
};
