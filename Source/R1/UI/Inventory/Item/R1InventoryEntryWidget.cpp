


#include "UI/Inventory/Item/R1InventoryEntryWidget.h"
#include "UI/Inventory/Item/R1ItemDragWidget.h"
#include "UI/Inventory/R1InventorySlotsWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"

#include "Item/R1InventorySubsystem.h"
#include "Item/R1ItemInstance.h"
#include "Item/R1DragDropOperation.h"
#include "Item/R1InventoryItemTooltipWidget.h"

UR1InventoryEntryWidget::UR1InventoryEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UR1ItemDragWidget> FindDragWidgetClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/Item/WBP_ItemDrag.WBP_ItemDrag_C'"));
	if (FindDragWidgetClass.Succeeded())
	{
		DragWidgetClass = FindDragWidgetClass.Class;
	}
}

void UR1InventoryEntryWidget::Init(UR1UserWidget* InSlotsWidget, UR1ItemInstance* InItemInstance, int32 InItemCount)
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

	if (ItemInstance && TooltipClass)
	{
		UR1InventoryItemTooltipWidget* TooltipWidget = CreateWidget<UR1InventoryItemTooltipWidget>(this, TooltipClass);
		if (TooltipWidget)
		{
			UR1InventorySubsystem* Inventory = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
			// 내 인벤토리에 좌표가 없다면(-1, -1) 상점 아이템으로 판별!
			bool bIsShopItem = (Inventory && Inventory->GetItemPosition(ItemInstance) == FIntPoint(-1, -1));

			TooltipWidget->SetupTooltip(ItemInstance, bIsShopItem);

			// 마우스를 올리면 자동으로 이 툴팁이 뜨게 만듭니다.
			SetToolTip(TooltipWidget);
		}
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

	FVector2D MouseWidgetPos = SlotsWidget->GetCachedGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	FVector2D ItemWidgetPos = SlotsWidget->GetCachedGeometry().AbsoluteToLocal(InGeometry.LocalToAbsolute(UnitSlotSize / 2.f));
	FIntPoint ItemSlotPos = FIntPoint(ItemWidgetPos.X / UnitSlotSize.X, ItemWidgetPos.Y / UnitSlotSize.Y);

	CachedFromSlotPos = ItemSlotPos;
	CachedDeltaWidgetPos = MouseWidgetPos - ItemWidgetPos;

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return Reply.DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	// 💡 2. 우클릭 분기 (거래 및 빠른 장착)
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		UR1InventorySubsystem* Inventory = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
		if (Inventory)
		{
			bool bIsShopItem = (Inventory->GetItemPosition(ItemInstance) == FIntPoint(-1, -1));

			// 🌟 A. [상점 -> 내 가방] 우클릭 시 즉시 구매!
			if (bIsShopItem && Inventory->bIsShopOpen)
			{
				Inventory->BuyItem(ItemInstance);
				// (필요하다면 여기서 상인 NPC의 ItemsForSale 배열에서 아이템을 지우는 로직을 호출할 수 있습니다)
				return FReply::Handled();
			}

			// 🌟 B. [내 가방 -> 상점] Shift + 우클릭 시 즉시 판매!
			if (!bIsShopItem && Inventory->bIsShopOpen && InMouseEvent.IsShiftDown())
			{
				Inventory->SellItem(ItemInstance, ItemInstance->ItemCount);
				return FReply::Handled();
			}

			// 🌟 C. 기존 로직: 내 가방에 있는 아이템을 우클릭했을 때 (빠른 장착)
			if (!bIsShopItem && !InMouseEvent.IsShiftDown())
			{
				TArray<ER1EquipmentSlot> CompatibleSlots = ItemInstance->GetEquipSlot();

				if (CompatibleSlots.Num() > 0)
				{
					ER1EquipmentSlot TargetSlot = ER1EquipmentSlot::None;

					for (ER1EquipmentSlot EnableSlot : CompatibleSlots)
					{
						if (Inventory->GetEquippedItem(EnableSlot) == nullptr)
						{
							TargetSlot = EnableSlot;
							break;
						}
					}

					if (TargetSlot == ER1EquipmentSlot::None)
					{
						TargetSlot = CompatibleSlots[0];
					}

					Inventory->EquipItem(ItemInstance, TargetSlot);
					Inventory->OnInventoryUpdated.Broadcast();
					return FReply::Handled();
				}
				// 장비가 아니라 포션 같은 소모품이라면 여기에 Consume 로직을 덧붙이면 됩니다!
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

