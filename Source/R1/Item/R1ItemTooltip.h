

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1ItemTooltip.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1ItemTooltip : public UR1UserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void SetItemInfo(const FText& InName, EItemRarity InRarity,int32 ItemCount, ER1ItemType InItemType);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_ItemName;
};
