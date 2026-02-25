

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1Define.h"
#include "R1EquipmentSlotWidget.generated.h"


/**
 * 
 */
UCLASS()
class R1_API UR1EquipmentSlotWidget : public UR1UserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	// 드래그할 때 마우스에 따라다닐 잔상 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "DragDrop")
	TSubclassOf<class UR1ItemDragWidget> DragWidgetClass;

protected:
	virtual void NativeConstruct() override;

public:
	// 💡 데이터가 변할 때마다 이 슬롯을 새로 그릴 함수
	UFUNCTION()
	void RefreshSlotUI();

public:
	// 💡 에디터에서 이 슬롯이 어떤 부위인지 설정할 수 있게 합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	ER1EquipmentSlot EquipmentSlotType = ER1EquipmentSlot::None;

	// 현재 이 슬롯에 장착된 아이템
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<class UR1ItemInstance> EquippedItem;

protected:
	// 슬롯의 물리적 크기를 결정할 사이즈 박스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USizeBox> SizeBox_Root;

	// 빈 칸일 때 보여줄 배경 이미지 (투구 모양 실루엣 등)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_Background;

	// 실제 아이템 아이콘이 표시될 이미지 (장착 시 활성화)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_ItemIcon;

	FVector2D CachedDragOffset = FVector2D::ZeroVector;
};
