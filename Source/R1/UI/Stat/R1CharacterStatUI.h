#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "GameplayTagContainer.h"
#include "R1CharacterStatUI.generated.h"

class UScrollBox;
class UTextBlock;
class UDataTable;
class UR1RunUpgradeComponent;

/**
 * Main Character Stat UI that handles run-specific stat upgrades and detail display.
 */
UCLASS()
class R1_API UR1CharacterStatUI : public UR1UserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "R1|UI")
	void RefreshUI();

protected:
	UFUNCTION()
	void HandleAvailablePointsChanged(int32 NewPoints);

	UFUNCTION()
	void HandleInvestmentHistoryChanged(FGameplayTag StatTag, int32 NewCount);

	/** Callback for when an upgrade button is clicked. */
	void OnUpgradeStatClicked(FGameplayTag StatTag);

protected:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox_UpgradeList;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox_DetailList;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_RemainPointAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_ClassName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_CurrentExp;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_ExpToLevelUp;

	UPROPERTY(EditDefaultsOnly, Category = "R1|UI")
	UDataTable* StatUpgradeDataTable;
};
