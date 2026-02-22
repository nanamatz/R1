


#include "UI/Inventory/R1InventorySlotsWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "UI/Inventory/R1InventroySlotWidget.h"
#include "Item/R1InventoryEntryWidget.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"
#include "Item/R1InventorySubsystem.h"
#include "Item/R1DragDropOperation.h"
#include "R1Define.h"
#include "Item/R1ItemInstance.h"

UR1InventorySlotsWidget::UR1InventorySlotsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UR1InventorySlotsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!GridPanel_Slots || !CanvasPanel_Entries || !SlotWidgetClass || !EntryWidgetClass)
	{
		return; // 하나라도 NULL이면 중단
	}
	UR1InventorySubsystem* Inventory = Cast<UR1InventorySubsystem>(USubsystemBlueprintLibrary::GetWorldSubsystem(this, UR1InventorySubsystem::StaticClass()));

	//UR1InventorySubsystem* Inventory = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (!Inventory) return;

	X_COUNT = Inventory->GetInventoryColumns();
	Y_COUNT = Inventory->GetInventoryRows();

	SlotWidgets.SetNum(X_COUNT * Y_COUNT);

	for (int32 y = 0; y < Y_COUNT; y++)
	{
		for (int32 x = 0; x < X_COUNT; x++)
		{
			int32 index = y * X_COUNT + x;

			UR1InventroySlotWidget* SlotWidget = CreateWidget<UR1InventroySlotWidget>(GetOwningPlayer(), SlotWidgetClass);
			if (SlotWidget == nullptr)
			{
				continue;
			}
			
			SlotWidgets[index] = SlotWidget;
			
			GridPanel_Slots->AddChildToUniformGrid(SlotWidget, y, x);
		}
	}

	EntryWidgets.SetNum(X_COUNT * Y_COUNT);

	//if (Inventory)
	//{
	//	const TArray<TObjectPtr<UR1ItemInstance>>& Items = Inventory->GetItems();

	//	for (int32 i = 0; i < Items.Num(); i++)
	//	{
	//		const TObjectPtr<UR1ItemInstance>& Item = Items[i];
	//		FIntPoint ItemSlotPos = FIntPoint(i % X_COUNT, i / X_COUNT);
	//		OnInventoryEntryChanged(ItemSlotPos, Item);
	//	}
	//}
	const TArray<TObjectPtr<UR1ItemInstance>>& Items = Inventory->GetItems();

	FIntPoint CurrentPos = FIntPoint(0, 0);

	for (UR1ItemInstance* Item : Items)
	{
		if (!Item) continue;

		// (단순화를 위해 순차적으로 넣습니다. 실전에서는 빈 칸을 찾는 알고리즘을 쓸 수도 있습니다.)
		if (Inventory->CanAddItemAt(Item->ItemSize, CurrentPos))
		{
			// UI 그리기
			OnInventoryEntryChanged(CurrentPos, Item);
			// 서브시스템(뇌)에 알박기 등록! (이게 있어야 겹침 판정이 제대로 돕니다)
			Inventory->AddItemToGrid(Item, CurrentPos);

			CurrentPos.X += Item->ItemSize.X; // 다음 아이템을 위해 X좌표 밀기
		}
	}
}

bool UR1InventorySlotsWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

	UR1DragDropOperation* DragDrop = Cast<UR1DragDropOperation>(InOperation);
	if (DragDrop == nullptr)
	{
		return false;
	}

	FVector2D MouseWidgetPos = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	FVector2D ToWidgetPos = MouseWidgetPos - DragDrop->DeltaWidgetPos;
	FIntPoint ToSlotPos = FIntPoint(ToWidgetPos.X / Item::UnitInventorySlotSize.X, ToWidgetPos.Y / Item::UnitInventorySlotSize.Y);


	if (PreDragOverSlotPos == ToSlotPos)
	{
		return true;
	}

	PreDragOverSlotPos = ToSlotPos;

	// (선택 사항) 여기에 마우스가 올라간 위치가 빨간색/초록색으로 빛나는 로직을 추가할 수 있습니다.

	return false;
}

void UR1InventorySlotsWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
	FinishDrag();
}

bool UR1InventorySlotsWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent,InOperation);
	FinishDrag();

	UR1DragDropOperation* DragDrop = Cast<UR1DragDropOperation>(InOperation);
	if (DragDrop == nullptr || !DragDrop->ItemInstance)
	{
		return false;
	}

	FVector2D MouseWidgetPos = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	FVector2D ToWidgetPos = MouseWidgetPos - DragDrop->DeltaWidgetPos;
	//FIntPoint ToItemSlotPos = FIntPoint(ToWidgetPos.X / Item::UnitInventorySlotSize.X, ToWidgetPos.Y / Item::UnitInventorySlotSize.Y);
	// 💡 Tip: 화면 밖으로 드래그했을 때 음수가 나오거나 배열을 벗어나는 것을 막기 위해 Clamp를 써주면 안전합니다.
	int32 TargetX = FMath::Clamp(FMath::FloorToInt(ToWidgetPos.X / Item::UnitInventorySlotSize.X), 0, X_COUNT - 1);
	int32 TargetY = FMath::Clamp(FMath::FloorToInt(ToWidgetPos.Y / Item::UnitInventorySlotSize.Y), 0, Y_COUNT - 1);
	FIntPoint ToItemSlotPos = FIntPoint(TargetX, TargetY);

	// 제자리에 놨으면 아무 일도 안 일어남
	if (DragDrop->FromItemSlotPos == ToItemSlotPos)
	{
		return false;
	}

	// 💡 서브시스템 문지기 등판! (겹침 검사)
	UR1InventorySubsystem* Inventory = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (Inventory)
	{
		// 이 자리에 놓을 수 있는지 확인 (자신(ItemInstance)은 겹침 검사에서 예외 처리)
		if (Inventory->CanAddItemAt(DragDrop->ItemInstance->ItemSize, ToItemSlotPos, DragDrop->ItemInstance))
		{
			// 1. 서브시스템의 실제 데이터 그리드(GridData) 위치 갱신 (아래에서 함수 추가 예정)
			Inventory->MoveItemInGrid(DragDrop->ItemInstance, DragDrop->FromItemSlotPos, ToItemSlotPos);

			// 2. UI 위젯 갱신
			OnInventoryEntryChanged(DragDrop->FromItemSlotPos, nullptr);
			OnInventoryEntryChanged(ToItemSlotPos, DragDrop->ItemInstance);

			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("여기는 겹치거나 공간이 부족해서 놓을 수 없습니다!"));
			// false를 반환하면 OnDragCancelled가 호출되면서 원래 자리로 아이템이 돌아갑니다.
			return false;
		}
	}

	return false;
}

void UR1InventorySlotsWidget::FinishDrag()
{
	PreDragOverSlotPos = FIntPoint(-1, -1);
}

void UR1InventorySlotsWidget::OnInventoryEntryChanged(const FIntPoint& InItemSlotPos, TObjectPtr<UR1ItemInstance> Item)
{
	int32 Slotindex = InItemSlotPos.Y * X_COUNT + InItemSlotPos.X;
	if (!EntryWidgets.IsValidIndex(Slotindex))
	{
		return;
	}

	if (UR1InventoryEntryWidget* EntryWidget = EntryWidgets[Slotindex])
	{
		if (Item == nullptr)
		{
			CanvasPanel_Entries->RemoveChild(EntryWidget);
			EntryWidgets[Slotindex] = nullptr;
		}
	}
	else
	{
		if (Item == nullptr)
		{
			return;
		}

		EntryWidget = CreateWidget<UR1InventoryEntryWidget>(GetOwningPlayer(), EntryWidgetClass);
		if (EntryWidget == nullptr)
		{
			return;
		}

		EntryWidgets[Slotindex] = EntryWidget;

		UCanvasPanelSlot* CanvasPanelSlot = CanvasPanel_Entries->AddChildToCanvas(EntryWidget);
		if (CanvasPanelSlot == nullptr)
		{
			EntryWidgets[Slotindex] = nullptr;
			return;
		}
		CanvasPanelSlot->SetAutoSize(true);
		CanvasPanelSlot->SetPosition(FVector2D(InItemSlotPos.X * 50, InItemSlotPos.Y * 50));

		//TODO
		EntryWidget->Init(this, Item, 1);
	}
}
