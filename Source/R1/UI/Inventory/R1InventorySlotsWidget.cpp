


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
	if (DragDrop == nullptr) return false;

	FVector2D MouseWidgetPos = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());

	// 1. 드래그 중 마우스가 밖으로 나가면 하이라이트를 지움
	float GridMaxWidth = X_COUNT * Item::UnitInventorySlotSize.X;
	float GridMaxHeight = Y_COUNT * Item::UnitInventorySlotSize.Y;
	if (MouseWidgetPos.X < 0 || MouseWidgetPos.X >= GridMaxWidth ||
		MouseWidgetPos.Y < 0 || MouseWidgetPos.Y >= GridMaxHeight)
	{
		FinishDrag(); // 밖으로 나가면 모든 색상 초기화!
		return false;
	}

	FVector2D ToWidgetPos = MouseWidgetPos - DragDrop->DeltaWidgetPos;
	int32 TargetX = FMath::FloorToInt(ToWidgetPos.X / Item::UnitInventorySlotSize.X);
	int32 TargetY = FMath::FloorToInt(ToWidgetPos.Y / Item::UnitInventorySlotSize.Y);

	FIntPoint ItemSize = DragDrop->ItemInstance->GetItemSize();
	TargetX = FMath::Clamp(TargetX, 0, X_COUNT - ItemSize.X);
	TargetY = FMath::Clamp(TargetY, 0, Y_COUNT - ItemSize.Y);

	FIntPoint ToSlotPos = FIntPoint(TargetX, TargetY);

	// 최적화: 마우스가 멈춰있거나 같은 칸 안에서만 움직일 땐 연산 생략
	if (PreDragOverSlotPos == ToSlotPos) return true;
	PreDragOverSlotPos = ToSlotPos;

	// ----------------------------------------------------------------------
	// 🌟 2. 하이라이트 칠하기 로직 시작
	// ----------------------------------------------------------------------

	// A. 일단 모든 슬롯의 상태를 노멀(투명)로 초기화합니다.
	for (UR1InventroySlotWidget* CurrentSlot : SlotWidgets)
	{
		if (CurrentSlot) CurrentSlot->SetSlotState(ESlotHoverState::Normal);
	}

	// B. 배치 가능 여부(초록/빨강) 검사
	UR1InventorySubsystem* Inventory = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (Inventory && DragDrop->ItemInstance)
	{
		bool bCanAdd = false;
		if (DragDrop->FromEquipmentSlot != ER1EquipmentSlot::None)
		{
			bCanAdd = Inventory->CanAddItemAt(ItemSize, ToSlotPos);
		}
		else
		{
			bCanAdd = Inventory->CanAddItemAt(ItemSize, ToSlotPos, DragDrop->ItemInstance);
		}

		ESlotHoverState State = bCanAdd ? ESlotHoverState::Valid : ESlotHoverState::Invalid;
		// C. 아이템 크기만큼 반복문을 돌면서 해당 칸들을 색칠합니다.
		for (int32 Y = 0; Y < ItemSize.Y; ++Y)
		{
			for (int32 X = 0; X < ItemSize.X; ++X)
			{
				int32 DrawX = ToSlotPos.X + X;
				int32 DrawY = ToSlotPos.Y + Y;

				if (DrawX >= 0 && DrawX < X_COUNT && DrawY >= 0 && DrawY < Y_COUNT)
				{
					int32 Index = DrawY * X_COUNT + DrawX;
					if (SlotWidgets.IsValidIndex(Index) && SlotWidgets[Index])
					{
						SlotWidgets[Index]->SetSlotState(State);
					}
				}
			}
		}
	}
	return true;
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

	// ----------------------------------------------------------------------
	// 🌟 1. [핵심] 마우스 포인터 자체가 인벤토리 그리드 바깥으로 나갔는가? (월드 드랍 판정)
	float GridMaxWidth = X_COUNT * Item::UnitInventorySlotSize.X;
	float GridMaxHeight = Y_COUNT * Item::UnitInventorySlotSize.Y;

	if (MouseWidgetPos.X < 0 || MouseWidgetPos.X >= GridMaxWidth ||
		MouseWidgetPos.Y < 0 || MouseWidgetPos.Y >= GridMaxHeight)
	{
		// 마우스가 화면 밖(허공)에 있으므로 바탕화면 위젯으로 이벤트를 넘겨 바닥에 버리게 합니다.
		return false;
	}

	// 🌟 2. 마우스가 인벤토리 '안'에 있다면, 아이템의 좌상단(Top-Left) 좌표 계산
	FVector2D ToWidgetPos = MouseWidgetPos - DragDrop->DeltaWidgetPos;
	int32 TargetX = FMath::FloorToInt(ToWidgetPos.X / Item::UnitInventorySlotSize.X);
	int32 TargetY = FMath::FloorToInt(ToWidgetPos.Y / Item::UnitInventorySlotSize.Y);

	// 🌟 3. [보정] 아이템이 인벤토리 벽을 뚫고 나가지 않게 강제로 안으로 밀어넣기! (Clamp 부활)
	// 예: 2x3 무기를 맨 오른쪽 끝에 놓으려 하면, X좌표를 8로 고정해서 안전하게 안착시킴
	FIntPoint ItemSize = DragDrop->ItemInstance->GetItemSize();
	TargetX = FMath::Clamp(TargetX, 0, X_COUNT - ItemSize.X);
	TargetY = FMath::Clamp(TargetY, 0, Y_COUNT - ItemSize.Y);

	FIntPoint ToItemSlotPos = FIntPoint(TargetX, TargetY);

	if (DragDrop->FromEquipmentSlot == ER1EquipmentSlot::None && DragDrop->FromItemSlotPos == ToItemSlotPos)
	{
		return false;
	}

	// 💡 서브시스템 문지기 등판! (겹침 검사)
	UR1InventorySubsystem* Inventory = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (Inventory)
	{
		// 💡 장비창에서 드래그 해온 경우 (장착 해제)
		if (DragDrop->FromEquipmentSlot != ER1EquipmentSlot::None)
		{
			// 인벤토리 바닥에 놓을 자리가 있는지 검사
			if (Inventory->CanAddItemAt(DragDrop->ItemInstance->GetItemSize(), ToItemSlotPos))
			{
				// 💡 서브시스템의 UnequipItem에 정확한 목표 위치를 전달! (내부에서 Items 추가, 그리드 알박기, 브로드캐스트까지 완료)
				Inventory->UnequipItem(DragDrop->FromEquipmentSlot, ToItemSlotPos);

				return true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("인벤토리에 장비를 벗어둘 공간이 부족합니다!"));
				return false;
			}
		}
		// 💡 기존 로직: 인벤토리 내부에서 이동한 경우
		else
		{
			if (Inventory->CanAddItemAt(DragDrop->ItemInstance->GetItemSize(), ToItemSlotPos, DragDrop->ItemInstance))
			{
				Inventory->MoveItemInGrid(DragDrop->ItemInstance, DragDrop->FromItemSlotPos, ToItemSlotPos);
				Inventory->OnInventoryUpdated.Broadcast(); // 이제 이 한 줄이면 UI가 다 갱신됩니다!
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	return false;
}

void UR1InventorySlotsWidget::FinishDrag()
{
	PreDragOverSlotPos = FIntPoint(-1, -1);

	// 🌟 드롭하거나 밖으로 나갈 때 잔상이 남지 않도록 무조건 초기화
	for (UR1InventroySlotWidget* CurrentSlot : SlotWidgets)
	{
		if (CurrentSlot)
		{
			CurrentSlot->SetSlotState(ESlotHoverState::Normal);
		}
	}
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

		EntryWidget->Init(this, Item, Item->ItemCount);
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
