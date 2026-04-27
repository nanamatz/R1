
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "R1MonsterInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1MonsterInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateMonsterInfo(const FString& Name, float HpRatio);

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	TObjectPtr<class UProgressBar> HpBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	TObjectPtr<class UTextBlock> MonsterName;
};
