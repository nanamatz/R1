


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
#include "Object/R1ItemActor.h"

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
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

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
		// [신규 분기] 🌟 외부(상점)에서 드래그 해온 경우 판별
		// 내 그리드 좌표에도 없고(-1, -1), 내 장비창에서도 오지 않았다면 상점 물건입니다.
		bool bIsFromShop = (Inventory->GetItemPosition(DragDrop->ItemInstance) == FIntPoint(-1, -1) && DragDrop->FromEquipmentSlot == ER1EquipmentSlot::None);

		if (bIsFromShop && Inventory->bIsShopOpen)
		{
			UR1ItemAssetData* Data = DragDrop->ItemInstance->GetItemData();
			int32 Price = Data->BaseValue * DragDrop->ItemInstance->ItemCount;

			// 1. 지갑 확인
			if (Inventory->Gold < Price)
			{
				UE_LOG(LogTemp, Warning, TEXT("골드가 부족하여 아이템을 구매할 수 없습니다. 필요: %d, 보유: %d"), Price, Inventory->Gold);
				return false;
			}

			// 2. 선 결제 진행 (돈부터 뺍니다)
			Inventory->ConsumeGold(Price);
			UE_LOG(LogTemp, Log, TEXT("상점 아이템 구매! (-%d 골드)"), Price);

			// 3. 드롭한 위치(ToItemSlotPos)에 자리가 있는지 최우선 확인
			if (Inventory->CanAddItemAt(ItemSize, ToItemSlotPos))
			{
				Inventory->Items.Add(DragDrop->ItemInstance);
				Inventory->AddItemToGrid(DragDrop->ItemInstance, ToItemSlotPos);
				Inventory->OnInventoryUpdated.Broadcast();
				return true;
			}
			else
			{
				// 4. 예외 처리 1: 드롭한 위치에 자리가 없다면, 가방 내 다른 빈 공간을 찾아본다.
				FIntPoint EmptyPos;
				if (Inventory->FindEmptySlot(ItemSize, EmptyPos))
				{
					Inventory->Items.Add(DragDrop->ItemInstance);
					Inventory->AddItemToGrid(DragDrop->ItemInstance, EmptyPos);
					Inventory->OnInventoryUpdated.Broadcast();
					UE_LOG(LogTemp, Warning, TEXT("지정한 위치에 자리가 없어 빈 공간에 자동으로 넣었습니다."));
					return true;
				}
				else
				{
					// 5. 예외 처리 2: 가방 전체가 꽉 찼다면 발밑에 드롭 (월드 스폰)
					UE_LOG(LogTemp, Warning, TEXT("인벤토리가 완전히 꽉 찼습니다! 아이템을 발밑에 떨어뜨립니다."));

					if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
					{
						if (APawn* PlayerPawn = PC->GetPawn())
						{
							FVector SpawnLocation = PlayerPawn->GetActorLocation();
							SpawnLocation.X += FMath::RandRange(-100.0f, 100.0f);
							SpawnLocation.Y += FMath::RandRange(-100.0f, 100.0f);
							SpawnLocation.Z += 50.0f;

							FActorSpawnParameters SpawnParams;
							SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

							AR1ItemActor* DroppedItemActor = GetWorld()->SpawnActor<AR1ItemActor>(AR1ItemActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
							if (DroppedItemActor)
							{
								DroppedItemActor->InitItem(Data, DragDrop->ItemInstance->ItemRarity, DragDrop->ItemInstance->ItemCount);
							}
						}
					}
					return true; // 어쨌든 유저가 돈을 내고 소유권이 넘어왔으므로 거래 자체는 true 반환
				}
			}
		}
		// 💡 기존 로직: 장비창에서 드래그 해온 경우 (장착 해제)
		else if (DragDrop->FromEquipmentSlot != ER1EquipmentSlot::None)
		{
			if (Inventory->CanAddItemAt(ItemSize, ToItemSlotPos))
			{
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
			if (Inventory->CanAddItemAt(ItemSize, ToItemSlotPos, DragDrop->ItemInstance))
			{
				Inventory->MoveItemInGrid(DragDrop->ItemInstance, DragDrop->FromItemSlotPos, ToItemSlotPos);
				Inventory->OnInventoryUpdated.Broadcast();
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

FReply UR1InventorySlotsWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	// 🌟 좌클릭이나 우클릭 시 여기서 이벤트를 '소모(Handled)'하여 
	// 마우스 클릭이 게임 월드(PlayerController)로 넘어가는 것을 완벽히 막습니다!
	return FReply::Handled();
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
