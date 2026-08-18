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

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetRunLevel(int32 InLevel) { RunLevel = InLevel; }

	UFUNCTION(BlueprintPure, Category = "Attributes")
	ER1CharacterClass GetPlayerClass() const { return PlayerClass; }

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetPlayerClass(ER1CharacterClass InClass) { PlayerClass = InClass; }

	// 메타 업그레이드 Revive 충전이 남아 있으면 1회 소모하고 true. 이번 런 동안 누적된 사용량으로 판정한다.
	// Revive 어트리뷰트 자체를 깎지 않는 이유: 메타 업그레이드 GE의 가산 모디파이어가 살아 있어
	// 애그리게이터가 재평가될 때마다 값이 되돌아오기 때문이다.
	bool TryConsumeRevive();

	UFUNCTION(BlueprintPure, Category = "Attributes")
	int32 GetRevivesUsed() const { return RevivesUsed; }

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	int32 RunLevel = 1;

	// 이번 런에서 소모한 부활 횟수.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	int32 RevivesUsed = 0;

	// Honor 보너스 강화 포인트를 이미 지급했는지 (ApplyMetaUpgrades가 두 번 불려도 중복 지급 방지).
	bool bHonorPointsGranted = false;

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
