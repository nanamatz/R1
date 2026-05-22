#pragma once

#include "CoreMinimal.h"
#include "R1LanguageTypes.generated.h"

UENUM(BlueprintType)
enum class ER1Language : uint8
{
    English = 0 UMETA(DisplayName = "English"),
    Korean  = 1 UMETA(DisplayName = "한국어")
};
