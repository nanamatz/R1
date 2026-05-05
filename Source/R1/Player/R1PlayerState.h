#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "R1Define.h"
#include "R1PlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExpChangedDelegate, float, Ratio);
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

	class UR1AbilitySystemComponent* GetR1AbilitySystemComponent() const;
	class UPlayerAttributeSet* GetPlayerAttributeSet() const;
	class UR1AttributeSet* GetCommonAttributeSet() const;
	class UR1RunUpgradeComponent* GetRunUpgradeComponent() const { return RunUpgradeComponent; }

public:
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UCurveTable> PlayerStatTable;

public:
	// 경험치 변경 시 UI에 알릴 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnExpChangedDelegate OnExpChanged;

	// UI 초기화를 위해 현재 경험치 비율을 반환하는 함수 (AttributeSet을 참조하여 계산)
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetCurrentExpRatio() const;

	UFUNCTION(BlueprintPure, Category = "Attributes")
	int32 GetRunLevel() const { return RunLevel; }

	UFUNCTION(BlueprintPure, Category = "Attributes")
	ER1CharacterClass GetPlayerClass() const { return PlayerClass; }

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetPlayerClass(ER1CharacterClass InClass) { PlayerClass = InClass; }

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	int32 RunLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	ER1CharacterClass PlayerClass = ER1CharacterClass::Knight;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<class UR1AbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr <class UR1AttributeSet > CoreAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr <class UPlayerAttributeSet > PlayerAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<class UR1RunUpgradeComponent> RunUpgradeComponent;

public:
	// 메타 업그레이드 스탯 적용 함수
	UFUNCTION(BlueprintCallable, Category = "AbilitySystem|Meta")
	void ApplyMetaUpgrades();

protected:
	// 에디터에서 할당할 데이터 테이블 (FR1MetaUpgradeData 구조체 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem|Meta")
	TObjectPtr<class UDataTable> MetaUpgradeDataTable;

	// 에디터에서 할당할 초기화 전용 Gameplay Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem|Meta")
	TSubclassOf<class UGameplayEffect> MetaUpgradeEffectClass;

	void OnLevelChanged(const struct FOnAttributeChangeData& Data);
};
