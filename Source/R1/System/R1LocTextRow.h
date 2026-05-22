#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "R1LocTextRow.generated.h"

USTRUCT(BlueprintType)
struct R1_API FR1LocTextRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Localization")
    FText English;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Localization")
    FText Korean;
};
