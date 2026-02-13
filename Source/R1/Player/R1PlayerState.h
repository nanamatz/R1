#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "R1PlayerState.generated.h"

class UAbilitySystemComponent;
class UR1AbilitySystemComponent;
class UR1PlayerAttributeSet;
/**
 * 
 */
UCLASS()
class R1_API AR1PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	 
public:
	AR1PlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;

	UR1AbilitySystemComponent* GetR1AbilitySystemComponent() const;
	UR1PlayerAttributeSet* GetR1PlayerAttributeSet() const;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	UCurveTable* PlayerStatTable;

protected:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<class UR1AbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr <UR1PlayerAttributeSet > PlayerAttributeSet;
};
