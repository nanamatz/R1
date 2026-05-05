#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1Define.h"
#include "GameplayTagContainer.h"
#include "R1CharacterStatUI.generated.h"

class UPanelWidget; // ScrollBox나 VerticalBox 모두 호환 가능
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
	FText GetCharacterClassName(ER1CharacterClass InClass) const;

	UFUNCTION()
	void HandleAvailablePointsChanged(int32 NewPoints);

	UFUNCTION()
	void HandleInvestmentHistoryChanged(FGameplayTag StatTag, int32 NewCount);

	UFUNCTION()
	void HandleExpChanged(float Ratio);

	UFUNCTION()
	void OnUpgradeStatClicked(FGameplayTag StatTag);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> List_Upgrade;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> List_Detail;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RemainPointAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ClassName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_CurrentExp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ExpToLevelUp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_LevelAmount;

	UPROPERTY(EditDefaultsOnly, Category = "R1|UI")
	TSubclassOf<class UR1StatUpgradeRow> UpgradeRowClass;

	UPROPERTY(EditDefaultsOnly, Category = "R1|UI")
	TSubclassOf<class UR1StatDetailRow> DetailRowClass;

	UPROPERTY(EditDefaultsOnly, Category = "R1|UI")
	TObjectPtr<UDataTable> StatUpgradeDataTable;
};
