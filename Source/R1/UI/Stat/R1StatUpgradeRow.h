

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

	void InjectData(const FText& InName, int32 InvestmentCount);
	
	void SetButtonEnabled(bool bEnabled);

	void SetStatTag(const FGameplayTag& InTag) { StatTag = InTag; }
	FGameplayTag GetStatTag() const { return StatTag; }

	class UButton* GetUpgradeButton() const { return Button_Upgrade; }

	UPROPERTY(BlueprintAssignable, Category = "R1|UI")
	FOnUpgradeRowClickedSignature OnUpgradeRowClicked;

public:
	// 기획자가 부모 위젯(CharacterStatUI)의 디테일 패널에서 직접 입력할 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "R1|UI Setup")
	FText EditorAttributeName;

	// C++에서 데이터 주입 짝을 찾을 때 사용할 함수
	FText GetAttributeNameText() const { return EditorAttributeName; }

protected:
	virtual void NativePreConstruct() override;

protected:
	UFUNCTION()
	void Internal_OnButtonClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_AttributeName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_StatValue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Upgrade;

private:
	FGameplayTag StatTag;
};
