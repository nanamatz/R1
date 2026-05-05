

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1StatDetailRow.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class R1_API UR1StatDetailRow : public UR1UserWidget
{
	GENERATED_BODY()

public:
	void InjectData(const FText& FormattedValue);
	FText GetAttributeName() const;

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_AttributeName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Amount;
};
