

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "R1GameMode.generated.h"

/**
 * 
 */
UCLASS()
class R1_API AR1GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

};