

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1StatUpgradeRow.generated.h"

class UTextBlock;
class UButton;

/**
 * 
 */
UCLASS()
class R1_API UR1StatUpgradeRow : public UR1UserWidget
{
	GENERATED_BODY()

public:
	void InjectData(int32 InvestmentCount);
	FText GetAttributeName() const;

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_AttributeName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_StatValue;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Upgrade;
};
