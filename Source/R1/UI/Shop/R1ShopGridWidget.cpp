#include "UI/Shop/R1ShopGridWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "UI/Inventory/R1InventroySlotWidget.h"
#include "UI/Shop/R1ShopEntryWidget.h"
#include "Item/R1ItemInstance.h"
#include "R1Define.h"

void UR1ShopGridWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!GridPanel_Slots || !CanvasPanel_Entries || !SlotWidgetClass || !EntryWidgetClass)
	{
		return;
	}

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
}

void UR1ShopGridWidget::InitShopGrid(const TArray<UR1ItemInstance*>& Items)
{
	if (!CanvasPanel_Entries || !EntryWidgetClass) return;

	CanvasPanel_Entries->ClearChildren();
	EntryWidgets.Empty();

	TArray<UR1ItemInstance*> LocalGridMap;
	LocalGridMap.Init(nullptr, X_COUNT * Y_COUNT);

	for (UR1ItemInstance* Item : Items)
	{
		if (!Item) continue;

		FIntPoint ItemSize = Item->GetItemSize();
		bool bPlaced = false;

		// 1. 빈 공간 찾기
		for (int32 y = 0; y <= Y_COUNT - ItemSize.Y && !bPlaced; ++y)
		{
			for (int32 x = 0; x <= X_COUNT - ItemSize.X && !bPlaced; ++x)
			{
				if (CanAddItemAt(ItemSize, FIntPoint(x, y), LocalGridMap))
				{
					// 2. 그리드에 점유 표시
					for (int32 iy = 0; iy < ItemSize.Y; ++iy)
					{
						for (int32 ix = 0; ix < ItemSize.X; ++ix)
						{
							int32 GridIndex = (y + iy) * X_COUNT + (x + ix);
							LocalGridMap[GridIndex] = Item;
						}
					}

					// 3. 엔트리 위젯 생성 및 캔버스에 추가
					UR1ShopEntryWidget* EntryWidget = CreateWidget<UR1ShopEntryWidget>(GetOwningPlayer(), EntryWidgetClass);
					if (EntryWidget)
					{
						UCanvasPanelSlot* CanvasPanelSlot = CanvasPanel_Entries->AddChildToCanvas(EntryWidget);
						if (CanvasPanelSlot)
						{
							CanvasPanelSlot->SetAutoSize(true);
							CanvasPanelSlot->SetPosition(FVector2D(x * Item::UnitInventorySlotSize.X, y * Item::UnitInventorySlotSize.Y));
						}

						EntryWidget->Init(Item);
						EntryWidgets.Add(EntryWidget);
					}

					bPlaced = true;
				}
			}
		}
	}
}

bool UR1ShopGridWidget::CanAddItemAt(const FIntPoint& ItemSize, const FIntPoint& TargetPos, const TArray<UR1ItemInstance*>& GridMap)
{
	if (TargetPos.X < 0 || TargetPos.Y < 0) return false;

	for (int32 X = 0; X < ItemSize.X; ++X)
	{
		for (int32 Y = 0; Y < ItemSize.Y; ++Y)
		{
			int32 CheckX = TargetPos.X + X;
			int32 CheckY = TargetPos.Y + Y;

			if (CheckX >= X_COUNT || CheckY >= Y_COUNT)
			{
				return false;
			}

			int32 GridIndex = CheckY * X_COUNT + CheckX;
			if (GridMap.IsValidIndex(GridIndex) && GridMap[GridIndex] != nullptr)
			{
				return false;
			}
		}
	}
	return true;
}
