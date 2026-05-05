#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "R1RunUpgradeComponent.generated.h"

class UGameplayEffect;
class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvailablePointsChangedSignature, int32, NewPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInvestmentHistoryChangedSignature, FGameplayTag, StatTag, int32, NewCount);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class R1_API UR1RunUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UR1RunUpgradeComponent();

	UFUNCTION(BlueprintCallable, Category = "Run Upgrade")
	void AddPoints(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Run Upgrade")
	void UpgradeStat(FGameplayTag StatTag);

	UFUNCTION(BlueprintCallable, Category = "Run Upgrade")
	void Reset();

	UFUNCTION(BlueprintPure, Category = "Run Upgrade")
	int32 GetInvestmentCount(FGameplayTag StatTag) const;

	UFUNCTION(BlueprintPure, Category = "Run Upgrade")
	int32 GetAvailablePoints() const { return AvailablePoints; }

	UFUNCTION(BlueprintPure, Category = "Run Upgrade")
	int32 GetPointsPerLevel() const { return PointsPerLevel; }

	UFUNCTION(BlueprintCallable, Category = "Run Upgrade")
	void ApplyRunUpgradeEffect();

	class UAbilitySystemComponent* GetAbilitySystemComponent() const;

protected:
	virtual void BeginPlay() override;
	void CacheDataTable();

public:
	UPROPERTY(BlueprintAssignable, Category = "Run Upgrade")
	FOnAvailablePointsChangedSignature OnAvailablePointsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Run Upgrade")
	FOnInvestmentHistoryChangedSignature OnInvestmentHistoryChanged;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Run Upgrade")
	int32 PointsPerLevel = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Run Upgrade")
	int32 AvailablePoints = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Run Upgrade")
	TMap<FGameplayTag, int32> InvestmentHistory;

	UPROPERTY(EditDefaultsOnly, Category = "Run Upgrade")
	TSubclassOf<UGameplayEffect> RunUpgradeGEClass;

	UPROPERTY(EditDefaultsOnly, Category = "Run Upgrade")
	UDataTable* StatUpgradeDataTable;

	UPROPERTY(Transient)
	TMap<FGameplayTag, struct FR1StatUpgradeData> CachedStatUpgradeData;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle RunUpgradeGEHandle;
};
