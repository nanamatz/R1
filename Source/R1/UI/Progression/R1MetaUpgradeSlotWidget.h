

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "GameplayTagContainer.h"
#include "R1MetaUpgradeSlotWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeButtonClicked, FGameplayTag, RequestedTag);
/**
 * 
 */
UCLASS()
class R1_API UR1MetaUpgradeSlotWidget : public UR1UserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	// 외부에서 이 슬롯의 데이터를 채워 넣을 때 호출하는 함수
	UFUNCTION(BlueprintCallable, Category = "MetaUI")
	void InitSlot(FGameplayTag InTag, const FText& InName, int32 InCurrentLevel, int32 InMaxLevel, bool bCanAfford);

	UPROPERTY(BlueprintAssignable, Category = "MetaUI")
	FOnUpgradeButtonClicked OnUpgradeButtonClicked;

protected:
	UFUNCTION()
	void OnButtonClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_SkillName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_Level;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Button_Upgrade;

private:
	FGameplayTag MyUpgradeTag;
};
