


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
		if (ItemInstance && ItemInstance->GetEquipSlot() != ER1EquipmentSlot::None)
		{
			UR1InventorySubsystem* Inventory = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
			if (Inventory)
			{
				// 스왑(교체)될 기존 장비가 있는지 미리 확인
				UR1ItemInstance* OldEquippedItem = Inventory->GetEquippedItem(ItemInstance->GetEquipSlot());

				// 1. 일단 내가 있던 자리의 인벤토리 그리드를 비워줌 (그래야 스왑된 템이 이 자리에 들어올 수 있음)
				Inventory->RemoveItemFromGrid(ItemInstance, ItemSlotPos);

				// 2. 장착 실행! (서브시스템 내부에서 알아서 교체/해제됨)
				Inventory->EquipItem(ItemInstance);

				// 3. 만약 원래 끼고 있던 장비가 튕겨져 나왔다면?
				if (OldEquippedItem)
				{
					// 내가 방금 비워준 자리에 들어갈 수 있으면 거기로 쏙!
					if (Inventory->CanAddItemAt(OldEquippedItem->GetItemSize(), ItemSlotPos))
					{
						Inventory->AddItemToGrid(OldEquippedItem, ItemSlotPos);
					}
					// 내 자리에 안 들어가면(크기가 다르면) 다른 빈칸 찾아서 넣기
					else
					{
						FIntPoint EmptyPos;
						if (Inventory->FindEmptySlot(OldEquippedItem->GetItemSize(), EmptyPos))
						{
							Inventory->AddItemToGrid(OldEquippedItem, EmptyPos);
						}
					}
				}

				// 4. UI 싹 다 갱신
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
	Text_Count->SetText((ItemCount >= 2) ? FText::AsNumber(ItemCount) : FText::GetEmpty());
}

