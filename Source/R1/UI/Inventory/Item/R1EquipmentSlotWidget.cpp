


#include "UI/Inventory/Item/R1EquipmentSlotWidget.h"
#include "UI/Inventory/Item/R1ItemDragWidget.h"

#include "Item/R1DragDropOperation.h"
#include "Item/R1ItemInstance.h"
#include "Item/R1InventorySubsystem.h"
#include "Item/R1InventoryItemTooltipWidget.h"

#include "Components/SizeBox.h"
#include "Components/Image.h"

#include "R1Define.h"
#include "Blueprint/WidgetBlueprintLibrary.h"



void UR1EquipmentSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (SizeBox_Root)
	{
		FVector2D UnitSlotSize = FVector2D(Item::UnitInventorySlotSize);	
		FIntPoint SlotSize = FIntPoint(1, 1);

		// 💡 부위에 따라 기본 빈 칸의 크기를 결정합니다!
		switch (EquipmentSlotType)
		{
		case ER1EquipmentSlot::Weapon:
		case ER1EquipmentSlot::Armor:
			SlotSize = FIntPoint(2, 3); // 무기, 갑옷은 2x3
			break;
		case ER1EquipmentSlot::Helmet:
		case ER1EquipmentSlot::Boots:
		case ER1EquipmentSlot::Glove:
			SlotSize = FIntPoint(2, 2); // 투구, 신발은 2x2
			break;
		case ER1EquipmentSlot::Ring1:
		case ER1EquipmentSlot::Ring2:
			SlotSize = FIntPoint(1, 1); // 반지는 1x1
			break;
		}

		SizeBox_Root->SetWidthOverride(SlotSize.X * UnitSlotSize.X);
		SizeBox_Root->SetHeightOverride(SlotSize.Y * UnitSlotSize.Y);
	}
}

bool UR1EquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	Image_Background->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));

	UR1DragDropOperation* DragDropOp = Cast<UR1DragDropOperation>(InOperation);
	if (!DragDropOp || !DragDropOp->ItemInstance) return false;

	// 💡 배열 안에 내가 속한 부위(EquipmentSlotType)가 있는지 검사!
	if (!DragDropOp->ItemInstance->GetEquipSlot().Contains(EquipmentSlotType))
	{
		UE_LOG(LogTemp, Warning, TEXT("착용할 수 없는 부위입니다!"));
		return false;
	}

	UR1InventorySubsystem* InventorySubsystem = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (InventorySubsystem)
	{
		// 💡 서브시스템의 EquipItem이 그리드 제거와 스왑을 모두 책임집니다!
		InventorySubsystem->EquipItem(DragDropOp->ItemInstance, EquipmentSlotType);

		return true;
	}

	return false;
}

FReply UR1EquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (EquippedItem == nullptr) return Reply;

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		CachedDragOffset = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		return Reply.DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	// 💡 2. 우클릭 (추가: 빠른 장착 해제)
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		UR1InventorySubsystem* Inventory = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
		if (Inventory)
		{
			// 💡 서브시스템의 UnequipItem이 자동으로 빈 자리를 찾아 그리드에 넣어줍니다.
			Inventory->UnequipItem(EquipmentSlotType);

			return FReply::Handled();
		}
	}
	return Reply;
}

void UR1EquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!EquippedItem || !DragWidgetClass) return;

	// 드래그 잔상 위젯 생성
	UR1ItemDragWidget* DragWidget = CreateWidget<UR1ItemDragWidget>(GetOwningPlayer(), DragWidgetClass);
	const FVector2D UnitSlotSize = Item::UnitInventorySlotSize;
	FVector2D EntityWidgetSize = FVector2D(EquippedItem->GetItemSize().X * UnitSlotSize.X, EquippedItem->GetItemSize().Y * UnitSlotSize.Y);

	DragWidget->Init(EntityWidgetSize, EquippedItem->GetItemIcon(), 1);

	// 드래그 정보(Operation) 세팅
	UR1DragDropOperation* DragDrop = NewObject<UR1DragDropOperation>();
	DragDrop->DefaultDragVisual = DragWidget;
	DragDrop->Pivot = EDragPivot::MouseDown;
	DragDrop->ItemInstance = EquippedItem;
	DragDrop->FromEquipmentSlot = EquipmentSlotType;
	DragDrop->DeltaWidgetPos = CachedDragOffset;

	OutOperation = DragDrop;

	// 드래그 중에는 원래 장비창의 아이콘을 반투명하게 만듦
	Image_ItemIcon->SetRenderOpacity(0.5f);
}

void UR1EquipmentSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	if (EquippedItem)
	{
		Image_ItemIcon->SetRenderOpacity(1.0f);
	}
}

void UR1EquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UR1InventorySubsystem* InventorySubsystem = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (InventorySubsystem)
	{
		// 1. 인벤토리/장비 데이터가 변할 때마다 내 RefreshSlotUI를 실행하라고 연결!
		InventorySubsystem->OnInventoryUpdated.AddDynamic(this, &UR1EquipmentSlotWidget::RefreshSlotUI);
	}

	// 2. UI가 처음 생성될 때, 이미 장착된 템이 있는지 확인하고 그리기
	RefreshSlotUI();
}

bool UR1EquipmentSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

	UR1DragDropOperation* DragDropOp = Cast<UR1DragDropOperation>(InOperation);
	if (!DragDropOp || !DragDropOp->ItemInstance) return false;

	// 💡 내가 속한 부위와 이 아이템의 부위가 일치하는가?
	if (DragDropOp->ItemInstance->GetEquipSlot().Contains(EquipmentSlotType))
	{
		// 초록색 (알파값 0.5로 반투명하게)
		Image_Background->SetColorAndOpacity(FLinearColor(0.0f, 1.0f, 0.0f, 0.5f));
	}
	else
	{
		// 빨간색 
		Image_Background->SetColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, 0.5f));
	}

	return true;
}

void UR1EquipmentSlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	// 🌟 마우스가 벗어나면 다시 원래 색(흰색)으로 복구
	Image_Background->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
}

void UR1EquipmentSlotWidget::RefreshSlotUI()
{
	UR1InventorySubsystem* InventorySubsystem = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (!InventorySubsystem) return;

	// 3. 서브시스템에서 내 부위(EquipmentSlotType)에 장착된 아이템을 가져옴
	UR1ItemInstance* LocalEquippedItem = InventorySubsystem->GetEquippedItem(EquipmentSlotType);

	if (LocalEquippedItem)
	{
		EquippedItem = LocalEquippedItem;

		// 💡 핵심: 구조체가 변경되었으므로 GetItemIcon()을 사용해야 합니다!
		if (UTexture2D* Icon = EquippedItem->GetItemIcon())
		{
			Image_ItemIcon->SetBrushFromTexture(Icon);
			Image_ItemIcon->SetRenderOpacity(1.0f);
			Image_ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

			Image_BGIcon->SetVisibility(ESlateVisibility::Hidden);
			Image_BGIcon->SetRenderOpacity(0.f);
		}

		if (TooltipClass)
		{
			UR1InventoryItemTooltipWidget* TooltipWidget = CreateWidget<UR1InventoryItemTooltipWidget>(this, TooltipClass);
			if (TooltipWidget)
			{
				// 상점이 아니고(false), 장착 중(true)이라고 알려줍니다.
				TooltipWidget->SetupTooltip(EquippedItem, false, true);
				SetToolTip(TooltipWidget);
			}
		}
	}
	else
	{
		// 4. 장착된 아이템이 없다면 아이콘 숨기기 (투명하게 만들고 터치 무시)
		EquippedItem = nullptr;
		Image_ItemIcon->SetRenderOpacity(0.0f);
		Image_ItemIcon->SetVisibility(ESlateVisibility::Hidden);

		Image_BGIcon->SetRenderOpacity(1.0f);
		Image_BGIcon->SetVisibility(ESlateVisibility::Visible);

		SetToolTip(nullptr);
	}
}
