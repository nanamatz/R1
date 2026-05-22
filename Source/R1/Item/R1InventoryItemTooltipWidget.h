

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1Define.h"
#include "R1InventoryItemTooltipWidget.generated.h"

class UTextBlock;
class UR1ItemInstance;
struct FGameplayTag;
/**
 * 
 */
UCLASS()
class R1_API UR1InventoryItemTooltipWidget : public UR1UserWidget
{
	GENERATED_BODY()

public:
	// bIsShopContext가 true면 '구매가(BaseValue)', false면 '판매가(GetSellPrice)'를 표시합니다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetupTooltip(UR1ItemInstance* ItemInstance, bool bIsShopContext, bool bIsEquipped = false);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void RefreshLocalization();

	TWeakObjectPtr<UR1ItemInstance> CachedItemInstance;
	bool bCachedIsShopContext = false;
	bool bCachedIsEquipped = false;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_IsEquipped;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ItemName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Rarity;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ItemType;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_EquipSlotType;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Price;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Skill;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UVerticalBox> VerticalBox_Stats;

	// 에디터에서 방금 만든 Row 위젯 클래스를 할당할 변수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UR1StatRowWidget> StatRowClass;
private:
	FString GetStatNameByTag(const FGameplayTag& Tag);
};
