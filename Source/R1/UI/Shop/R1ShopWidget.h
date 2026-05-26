

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1ShopWidget.generated.h"

class UR1ShopSlotsWidget;
class UButton;
class AR1MerchantNPC;
class UR1ShopNPCData;
/**
 * 
 */
UCLASS()
class R1_API UR1ShopWidget : public UR1UserWidget
{
	GENERATED_BODY()

public:
	UR1ShopWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// HUD에서 호출하여 상점 데이터를 주입합니다.
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void InitShop(AR1MerchantNPC* InNPC);
	void InitShopNPC(class UR1ShopNPCData* NPCData);
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	void RefreshLocalization();

	// 닫기 버튼 클릭 시 호출
	UFUNCTION()
	void OnCloseButtonClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UR1ShopSlotsWidget> ShopSlotsWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Close;
	UPROPERTY(meta = (BindWidget))

	TObjectPtr<class UImage> Image_NPC_Portrait;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_ShopNPC;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_Greeting;

	UPROPERTY()
	TObjectPtr<UR1ShopNPCData> CachedNPCData;

	// 랜덤으로 선택된 인사말 인덱스 (-1 = 미선택)
	int32 CachedGreetingIndex = -1;
};
