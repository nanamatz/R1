#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "R1PlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExpChangedDelegate, float, Ratio);

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

public:
	// 경험치 변경 시 UI에 알릴 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnExpChangedDelegate OnExpChanged;

	// UI 초기화를 위해 현재 경험치 비율을 반환하는 함수 (AttributeSet을 참조하여 계산)
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetCurrentExpRatio() const;

protected:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<class UR1AbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr <UR1PlayerAttributeSet > PlayerAttributeSet;
};
