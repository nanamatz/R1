#pragma once

#include "CoreMinimal.h"
#include "R1LanguageTypes.generated.h"

UENUM(BlueprintType)
enum class ELanguage : uint8
{
    English UMETA(DisplayName = "English"),
    Korean  UMETA(DisplayName = "한국어")
};
