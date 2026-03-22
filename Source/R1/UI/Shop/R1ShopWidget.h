

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1ShopWidget.generated.h"

class UR1ShopSlotsWidget;
class UButton;
class AR1MerchantNPC;
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

protected:
	virtual void NativeConstruct() override;

	// 닫기 버튼 클릭 시 호출
	UFUNCTION()
	void OnCloseButtonClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UR1ShopSlotsWidget> ShopSlotsWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Close;
};
