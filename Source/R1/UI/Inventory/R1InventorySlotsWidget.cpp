


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

	if (!Inventory) return;

	Inventory->OnInventoryUpdated.AddDynamic(this, &UR1InventorySlotsWidget::RefreshInventoryUI);

	X_COUNT = Inventory->GetInventoryColumns();
	Y_COUNT = Inventory->GetInventoryRows();

	SlotWidgets.SetNum(X_COUNT * Y_COUNT);
	EntryWidgets.SetNum(X_COUNT * Y_COUNT);

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

	RefreshInventoryUI();
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
		if (Inventory->CanAddItemAt(DragDrop->ItemInstance->GetItemSize(), ToItemSlotPos, DragDrop->ItemInstance))
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

void UR1InventorySlotsWidget::RefreshInventoryUI()
{
	UR1InventorySubsystem* Inventory = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (!Inventory) return;

	// 1. 기존 화면에 그려진 아이템들을 싹 다 지웁니다.
	if (CanvasPanel_Entries)
	{
		CanvasPanel_Entries->ClearChildren();
	}
	// 배열 포인터들도 깔끔하게 초기화합니다.
	EntryWidgets.Init(nullptr, X_COUNT * Y_COUNT);

	// 2. 서브시스템의 최신 데이터를 가져옵니다.
	const TArray<TObjectPtr<UR1ItemInstance>>& GridData = Inventory->GetGridData();

	// 💡 중복 그리기 방지용 Set (2x3 아이템은 배열의 6칸을 차지하므로 한 번만 그려야 함)
	TSet<UR1ItemInstance*> DrawnItems;

	// 3. 데이터를 순회하며 화면에 그립니다.
	for (int32 y = 0; y < Y_COUNT; y++)
	{
		for (int32 x = 0; x < X_COUNT; x++)
		{
			int32 Index = y * X_COUNT + x;

			// 배열 범위 체크 방어코드
			if (!GridData.IsValidIndex(Index)) continue;

			UR1ItemInstance* Item = GridData[Index];

			// 칸에 아이템이 존재하고, 아직 화면에 안 그렸다면?
			if (Item && !DrawnItems.Contains(Item))
			{
				// 좌상단(Top-Left) 기준점에 아이템 UI를 생성!
				OnInventoryEntryChanged(FIntPoint(x, y), Item);

				// 그렸다고 메모해 둠 (다음 칸에서 중복으로 안 그리게)
				DrawnItems.Add(Item);
			}
		}
	}
}
