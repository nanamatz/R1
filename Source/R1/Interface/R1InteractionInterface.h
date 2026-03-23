
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "R1InteractionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UR1InteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class R1_API IR1InteractionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 플레이어가 도착했을 때 실행될 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(class AR1PlayerController* Interactor);

	virtual UPrimitiveComponent* GetInteractTrigger() { return nullptr; }
};
