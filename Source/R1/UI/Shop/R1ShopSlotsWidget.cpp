


#include "UI/Shop/R1ShopSlotsWidget.h"
#include "UI/Inventory/R1InventroySlotWidget.h"
#include "UI/Inventory/Item/R1InventoryEntryWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Item/R1ItemInstance.h"
#include "Item/R1InventorySubsystem.h"
#include "Object/R1MerchantNPC.h"
#include "Item/R1DragDropOperation.h"

UR1ShopSlotsWidget::UR1ShopSlotsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UR1ShopSlotsWidget::InitShopGrid(AR1MerchantNPC* InNPC)
{
	CurrentNPC = InNPC;
	RefreshShopUI();
}

void UR1ShopSlotsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!GridPanel_Slots || !SlotWidgetClass) return;

	GridPanel_Slots->ClearChildren();
	SlotWidgets.Empty();

	// 1. 6x3 사이즈의 빈 슬롯 배경을 생성하여 UniformGridPanel에 채워 넣습니다.
	for (int32 y = 0; y < Y_COUNT; ++y)
	{
		for (int32 x = 0; x < X_COUNT; ++x)
		{
			UR1InventroySlotWidget* NewSlot = CreateWidget<UR1InventroySlotWidget>(this, SlotWidgetClass);
			if (NewSlot)
			{
				// UniformGridPanel에 자식으로 추가하고 반환된 Slot 객체를 통해 행/열을 지정합니다.
				UUniformGridSlot* GridSlot = GridPanel_Slots->AddChildToUniformGrid(NewSlot);
				if (GridSlot)
				{
					GridSlot->SetRow(y);
					GridSlot->SetColumn(x);
				}
				SlotWidgets.Add(NewSlot);
			}
		}
	}
}

bool UR1ShopSlotsWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 상점 위를 드래그할 때의 처리를 이곳에 작성합니다. (현재는 통과만 허용)
	return true;
}

bool UR1ShopSlotsWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	if (!InOperation || !CurrentNPC) return false;

	// 🌟 유저님이 만드신 UR1DragDropOperation으로 캐스팅
	UR1DragDropOperation* DragDrop = Cast<UR1DragDropOperation>(InOperation);
	if (!DragDrop || !DragDrop->ItemInstance) return false;

	UR1ItemInstance* DroppedItem = DragDrop->ItemInstance;

	UR1InventorySubsystem* InvenSubsys = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (!InvenSubsys) return false;

	// 출처 판별: 상점 NPC의 ItemsForSale 배열에 이 아이템이 없다면, 플레이어가 던진 것(판매)입니다.
	bool bIsFromPlayer = !CurrentNPC->GetItemsForSale().Contains(DroppedItem);

	if (bIsFromPlayer)
	{
		// 💰 [판매 로직] 
		// (주의: ItemInstance에 GetTotalValue()가 아직 없다면 Data->BaseValue * Count 로 우회합니다)
		int32 SellPrice = 0;
		if (DroppedItem->GetItemData())
		{
			int32 UnitValue = DroppedItem->GetItemData()->BaseValue;
			SellPrice = FMath::Max(1, FMath::FloorToInt(UnitValue * 0.7f)) * DroppedItem->ItemCount;
		}

		// 방금 새로 만든 RemoveItem을 사용하여 인벤토리에서 제거합니다.
		if (InvenSubsys->RemoveItem(DroppedItem))
		{
			InvenSubsys->AddGold(SellPrice);
			UE_LOG(LogTemp, Log, TEXT("아이템 판매 완료! +%d 골드"), SellPrice);
			return true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("상점 내에서는 아이템을 이동시킬 수 없습니다."));
		return false;
	}

	return false;
}

void UR1ShopSlotsWidget::RefreshShopUI()
{
	if (!CurrentNPC || !CanvasPanel_Entries || !EntryWidgetClass) return;

	CanvasPanel_Entries->ClearChildren();
	EntryWidgets.Empty();

	const float SlotPixelSize = Item::UnitInventorySlotSize.X; // 🌟 유저님의 유닛 슬롯 사이즈 사용

	// 💡 상점 내 빈칸 배치를 위한 임시 1차원 배열(그리드 맵)을 생성합니다.
	TArray<bool> ShopGridMap;
	ShopGridMap.Init(false, X_COUNT * Y_COUNT);

	for (UR1ItemInstance* ShopItem : CurrentNPC->GetItemsForSale())
	{
		if (!ShopItem) continue;

		UR1InventoryEntryWidget* NewEntry = CreateWidget<UR1InventoryEntryWidget>(this, EntryWidgetClass);
		if (NewEntry)
		{
			NewEntry->Init(this, ShopItem, ShopItem->ItemCount);

			UCanvasPanelSlot* CanvasSlot = CanvasPanel_Entries->AddChildToCanvas(NewEntry);
			if (CanvasSlot)
			{
				CanvasSlot->SetAutoSize(true);

				// 🌟 빈 칸(테트리스 공간) 찾기 로직
				FIntPoint ItemSize = ShopItem->GetItemSize();
				FIntPoint FoundPos(0, 0);
				bool bFound = false;

				for (int32 y = 0; y <= Y_COUNT - ItemSize.Y; ++y)
				{
					for (int32 x = 0; x <= X_COUNT - ItemSize.X; ++x)
					{
						bool bCanFit = true;
						// 해당 좌표에 아이템 사이즈만큼 빈 공간이 있는지 검사
						for (int32 iy = 0; iy < ItemSize.Y; ++iy)
						{
							for (int32 ix = 0; ix < ItemSize.X; ++ix)
							{
								if (ShopGridMap[(y + iy) * X_COUNT + (x + ix)])
								{
									bCanFit = false;
									break;
								}
							}
							if (!bCanFit) break;
						}

						// 빈 공간을 찾았다면 해당 구역을 '점유 상태'로 마킹하고 좌표를 기록
						if (bCanFit)
						{
							for (int32 iy = 0; iy < ItemSize.Y; ++iy)
							{
								for (int32 ix = 0; ix < ItemSize.X; ++ix)
								{
									ShopGridMap[(y + iy) * X_COUNT + (x + ix)] = true;
								}
							}
							FoundPos = FIntPoint(x, y);
							bFound = true;
							break;
						}
					}
					if (bFound) break;
				}

				// 찾은 빈칸 좌표를 픽셀 단위로 변환하여 캔버스에 배치합니다.
				FVector2D ItemPixelPos(FoundPos.X * SlotPixelSize, FoundPos.Y * SlotPixelSize);
				CanvasSlot->SetPosition(ItemPixelPos);
			}

			EntryWidgets.Add(NewEntry);
		}
	}
}
