

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1Define.h"
#include "R1InventroySlotWidget.generated.h"


class UImage;
class USizeBox;

UENUM(BlueprintType)
enum class ESlotHoverState : uint8
{
	Normal		UMETA(DisplayName = "기본 상태"),
	Valid		UMETA(DisplayName = "배치 가능 (초록)"),
	Invalid		UMETA(DisplayName = "배치 불가 (빨강)")
};
/**
 * 
 */
UCLASS()
class R1_API UR1InventroySlotWidget : public UR1UserWidget
{
	GENERATED_BODY()
	
public:
	UR1InventroySlotWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;

public:
	// 🌟 상태에 따라 배경색을 바꿔줄 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetSlotState(ESlotHoverState InState);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetItemRarity(bool bHasItem, EItemRarity InRarity = EItemRarity::Common);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Colors")
	FLinearColor NormalColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Colors")
	FLinearColor ValidColor = FLinearColor(0.15f, 1.0f, 0.15f, 0.8f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Colors")
	FLinearColor InvalidColor = FLinearColor(1.0f, 0.1f, 0.1f, 0.8f);

protected:
	// 🌟 내부적으로 우선순위를 따져서 최종 색상을 칠하는 함수
	void RefreshColor();

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Background;

private:
	// 현재 슬롯의 상태를 기억할 변수들
	ESlotHoverState CurrentHoverState = ESlotHoverState::Normal;
	bool bIsOccupied = false;
	EItemRarity OccupiedRarity = EItemRarity::Common;
};
