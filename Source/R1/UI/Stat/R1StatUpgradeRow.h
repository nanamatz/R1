

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "GameplayTagContainer.h"
#include "R1StatUpgradeRow.generated.h"

class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeRowClickedSignature, FGameplayTag, StatTag);

/**
 * 
 */
UCLASS()
class R1_API UR1StatUpgradeRow : public UR1UserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void InjectData(int32 InvestmentCount);
	FText GetAttributeName() const;

	void SetStatTag(const FGameplayTag& InTag) { StatTag = InTag; }
	FGameplayTag GetStatTag() const { return StatTag; }

	class UButton* GetUpgradeButton() const { return Button_Upgrade; }

	UPROPERTY(BlueprintAssignable, Category = "R1|UI")
	FOnUpgradeRowClickedSignature OnUpgradeRowClicked;

protected:
	UFUNCTION()
	void Internal_OnButtonClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_AttributeName;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_StatValue;

	UPROPERTY(meta = (BindWidget))
	class UButton* Button_Upgrade;

private:
	FGameplayTag StatTag;
};
