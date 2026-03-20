


#include "UI/Inventory/Item/R1InventoryEntryWidget.h"
#include "UI/Inventory/Item/R1ItemDragWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "UI/Inventory/R1InventorySlotsWidget.h"
#include "Item/R1InventorySubsystem.h"
#include "Item/R1ItemInstance.h"
#include "Item/R1DragDropOperation.h"

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

	RefreshItemCount(InItemCount);

	if (ItemInstance && SizeBox_Root)
	{
		// 💡 1. 아이콘 이미지 씌우기
		if (UTexture2D* IconTex = ItemInstance->GetItemIcon())
		{
			Image_Icon->SetBrushFromTexture(IconTex);
		}

		const FVector2D UnitSlotSize = Item::UnitInventorySlotSize;

		float WidgetWidth = ItemInstance->GetItemSize().X * UnitSlotSize.X;
		float WidgetHeight = ItemInstance->GetItemSize().Y * UnitSlotSize.Y;

		SizeBox_Root->SetWidthOverride(WidgetWidth);
		SizeBox_Root->SetHeightOverride(WidgetHeight);
	}
	if (Image_Hover && !IsHovered())
	{
		Image_Hover->SetRenderOpacity(0.f);
	}
}

void UR1InventoryEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Text_Count)
	{
		RefreshItemCount(ItemCount);
	}

	if (Image_Hover)
	{
		Image_Hover->SetRenderOpacity(0.f);
	}
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

	const FVector2D UnitSlotSize = FVector2D(Item::UnitInventorySlotSize);

	//마우스 커서 위치에 따라 변환 및 계산
	FVector2D MouseWidgetPos = SlotsWidget->GetCachedGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	FVector2D ItemWidgetPos = SlotsWidget->GetCachedGeometry().AbsoluteToLocal(InGeometry.LocalToAbsolute(UnitSlotSize / 2.f));
	FIntPoint ItemSlotPos = FIntPoint(ItemWidgetPos.X / UnitSlotSize.X, ItemWidgetPos.Y / UnitSlotSize.Y);

	CachedFromSlotPos = ItemSlotPos;
	CachedDeltaWidgetPos = MouseWidgetPos - ItemWidgetPos;

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return Reply.DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	// 💡 2. 우클릭 (추가: 빠른 장착)
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		TArray<ER1EquipmentSlot> CompatibleSlots = ItemInstance->GetEquipSlot();

		// 장착 가능한 부위가 하나라도 있다면
		if (CompatibleSlots.Num() > 0)
		{
			UR1InventorySubsystem* Inventory = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
			if (Inventory)
			{
				ER1EquipmentSlot TargetSlot = ER1EquipmentSlot::None;

				// 💡 1. 호환되는 슬롯들 중에 "비어있는" 슬롯을 우선적으로 찾습니다.
				for (ER1EquipmentSlot EnableSlot : CompatibleSlots)
				{
					if (Inventory->GetEquippedItem(EnableSlot) == nullptr)
					{
						TargetSlot = EnableSlot;
						break; // 빈자리 발견! 루프 종료
					}
				}

				// 💡 2. 만약 호환 슬롯이 모두 꽉 차 있다면? -> 그냥 배열의 첫 번째 슬롯(주력 슬롯)을 덮어씌움!
				if (TargetSlot == ER1EquipmentSlot::None)
				{
					TargetSlot = CompatibleSlots[0];
				}

				// 💡 서브시스템의 EquipItem이 그리드 제거와 스왑을 모두 책임지도록 변경!
				Inventory->EquipItem(ItemInstance, TargetSlot);

				Inventory->OnInventoryUpdated.Broadcast();
				return FReply::Handled();
			}
		}
	}

	return Reply;
}

void UR1InventoryEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!ItemInstance) return;

	UR1ItemDragWidget* DragWidget = CreateWidget<UR1ItemDragWidget>(GetOwningPlayer(), DragWidgetClass);
	const FVector2D UnitSlotSize = FVector2D(Item::UnitInventorySlotSize);

	FVector2D EntityWidgetSize = FVector2D(ItemInstance->GetItemSize().X * UnitSlotSize.X, ItemInstance->GetItemSize().Y * UnitSlotSize.Y);
	
	DragWidget->Init(EntityWidgetSize, ItemInstance->GetItemIcon(), ItemCount);

	UR1DragDropOperation* DragDrop = NewObject<UR1DragDropOperation>();

	DragDrop->DefaultDragVisual = DragWidget;
	DragDrop->Pivot = EDragPivot::MouseDown;
	DragDrop->FromItemSlotPos = CachedFromSlotPos;
	DragDrop->ItemInstance = ItemInstance;
	DragDrop->DeltaWidgetPos = CachedDeltaWidgetPos;

	OutOperation = DragDrop;

	// 인벤토리 바닥에 남은 내 자신은 반투명하게 만듦
	RefreshWidgetOpacity(false);
}

void UR1InventoryEntryWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	RefreshWidgetOpacity(true);
}

void UR1InventoryEntryWidget::RefreshWidgetOpacity(bool bClearVisible)
{
	SetRenderOpacity(bClearVisible ? 1.f : 0.5f);
}

void UR1InventoryEntryWidget::RefreshItemCount(int32 NewItemCount)
{
	ItemCount = NewItemCount;
	if (Text_Count)
	{
		Text_Count->SetText((ItemCount >= 2) ? FText::AsNumber(ItemCount) : FText::GetEmpty());
	}
}

