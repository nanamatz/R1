


#include "Item/R1InventoryEntryWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "UI/Inventory/R1InventorySlotsWidget.h"
#include "Item/R1InventorySubsystem.h"
#include "Item/R1ItemInstance.h"
#include "Item/R1DragDropOperation.h"
#include "Item/R1ItemDragWidget.h"

UR1InventoryEntryWidget::UR1InventoryEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UR1ItemDragWidget> FindDragWidgetClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/Item/WBP_ItemDrag.WBP_ItemDrag_C'"));
	if (FindDragWidgetClass.Succeeded())
	{
		DragWidgetClass = FindDragWidgetClass.Class;
	}
}

void UR1InventoryEntryWidget::Init(UR1InventorySlotsWidget* InSlotsWidget, UR1ItemInstance* InItemInstance, int32 InItemCount)
{
	SlotsWidget = InSlotsWidget;
	ItemInstance = InItemInstance;
	ItemCount = InItemCount;

	if (ItemInstance && SizeBox_Root)
	{
		const FVector2D UnitSlotSize = Item::UnitInventorySlotSize;

		float WidgetWidth = ItemInstance->GetItemSize().X * UnitSlotSize.X;
		float WidgetHeight = ItemInstance->GetItemSize().Y * UnitSlotSize.Y;

		SizeBox_Root->SetWidthOverride(WidgetWidth);
		SizeBox_Root->SetHeightOverride(WidgetHeight);
	}
}

void UR1InventoryEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Text_Count->SetText(FText::GetEmpty());	//별도의 함수로 빼는 게 좋음
}

void UR1InventoryEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	
	Image_Hover->SetRenderOpacity(1.f);
}

void UR1InventoryEntryWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	Image_Hover->SetRenderOpacity(0.f);
}

FReply UR1InventoryEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		Reply.DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	/*const FIntPoint UnitInventorySlotSize = FIntPoint(50, 50);*/
	const FVector2D UnitSlotSize = FVector2D(Item::UnitInventorySlotSize);

	//마우스 커서 위치에 따라 변환 및 계산
	FVector2D MouseWidgetPos = SlotsWidget->GetCachedGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	FVector2D ItemWidgetPos = SlotsWidget->GetCachedGeometry().AbsoluteToLocal(InGeometry.LocalToAbsolute(UnitSlotSize / 2.f));
	FIntPoint ItemSlotPos = FIntPoint(ItemWidgetPos.X / UnitSlotSize.X, ItemWidgetPos.Y / UnitSlotSize.Y);
	//FVector2D ItemWidgetPos = SlotsWidget->GetCachedGeometry().AbsoluteToLocal(InGeometry.LocalToAbsolute(FVector2D::ZeroVector));
	//FIntPoint ItemSlotPos = FIntPoint(FMath::RoundToInt(ItemWidgetPos.X / UnitSlotSize.X), FMath::RoundToInt(ItemWidgetPos.Y / UnitSlotSize.Y));

	CachedFromSlotPos = ItemSlotPos;
	CachedDeltaWidgetPos = MouseWidgetPos - ItemWidgetPos;

	return Reply;
}

void UR1InventoryEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!ItemInstance) return; // 안전 검사

	UR1ItemDragWidget* DragWidget = CreateWidget<UR1ItemDragWidget>(GetOwningPlayer(), DragWidgetClass);
	const FVector2D UnitSlotSize = FVector2D(Item::UnitInventorySlotSize);

	FVector2D EntityWidgetSize = FVector2D(ItemInstance->GetItemSize().X * UnitSlotSize.X, ItemInstance->GetItemSize().Y * UnitSlotSize.Y);
	
	//TODO (나중에 nullptr 대신 아이템의 텍스처/아이콘을 넘기도록 수정해야 합니다)
	DragWidget->Init(EntityWidgetSize, nullptr, ItemCount);

	UR1DragDropOperation* DragDrop = NewObject<UR1DragDropOperation>();

	DragDrop->DefaultDragVisual = DragWidget;
	DragDrop->Pivot = EDragPivot::MouseDown;
	DragDrop->FromItemSlotPos = CachedFromSlotPos;
	DragDrop->ItemInstance = ItemInstance;
	DragDrop->DeltaWidgetPos = CachedDeltaWidgetPos;

	OutOperation = DragDrop;
}

void UR1InventoryEntryWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	// 서브시스템을 확인해서, 내 아이템이 인벤토리 배열(Items)에서 사라졌는지 확인
	UR1InventorySubsystem* InventorySubsystem = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (InventorySubsystem && !InventorySubsystem->GetItems().Contains(ItemInstance))
	{
		// 인벤토리에서 이미 지워졌다면(장착되었다면), 나 자신(UI 위젯)도 화면에서 삭제!
		RemoveFromParent();
	}
	else
	{
		// 드롭에 실패해서 그냥 인벤토리에 남아있다면, 다시 불투명도를 100%로 복구
		RefreshWidgetOpacity(true);
	}
}

void UR1InventoryEntryWidget::RefreshWidgetOpacity(bool bClearVisible)
{
	SetRenderOpacity(bClearVisible ? 1.f : 0.5f);
}

void UR1InventoryEntryWidget::RefreshItemCount(int32 NewItemCount)
{
	ItemCount = NewItemCount;
	Text_Count->SetText((ItemCount >= 2) ? FText::AsNumber(ItemCount) : FText::GetEmpty());
}

