


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

	if (!GridPanel_Slots || !SlotWidgetClass || !CanvasPanel_Entries || !EntryWidgetClass) return;

	//GridPanel_Slots->ClearChildren();
	//SlotWidgets.Empty();

	SlotWidgets.SetNum(X_COUNT * Y_COUNT);
	EntryWidgets.SetNum(X_COUNT * Y_COUNT);

	for (int32 y = 0; y < Y_COUNT; ++y)
	{
		for (int32 x = 0; x < X_COUNT; ++x)
		{
			int32 index = y * X_COUNT + x;

			// 🌟 수정 1: this 대신 GetOwningPlayer()를 사용하여 DPI 스케일링 문제를 차단합니다.
			UR1InventroySlotWidget* SlotWidget = CreateWidget<UR1InventroySlotWidget>(GetOwningPlayer(), SlotWidgetClass);
			if (SlotWidget == nullptr)
			{
				continue;
			}
			SlotWidgets[index] = SlotWidget;

			GridPanel_Slots->AddChildToUniformGrid(SlotWidget, y, x);
		}
	}
	RefreshShopUI();
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

	// 🌟 [수정 1] 인벤토리와 동일하게 EntryWidgets 배열을 그리드 크기만큼 꽉 채워서 초기화
	EntryWidgets.Init(nullptr, X_COUNT * Y_COUNT);

	// 상점 내 빈칸 배치를 위한 임시 1차원 배열(그리드 맵)
	TArray<bool> ShopGridMap;
	ShopGridMap.Init(false, X_COUNT * Y_COUNT);

	for (UR1ItemInstance* ShopItem : CurrentNPC->GetItemsForSale())
	{
		if (!ShopItem) continue;

		FIntPoint ItemSize = ShopItem->GetItemSize();
		FIntPoint FoundPos(0, 0);
		bool bFound = false;

		// --- [빈 공간 찾는 테트리스 로직 유지] ---
		for (int32 y = 0; y <= Y_COUNT - ItemSize.Y; ++y)
		{
			for (int32 x = 0; x <= X_COUNT - ItemSize.X; ++x)
			{
				bool bCanFit = true;
				for (int32 iy = 0; iy < ItemSize.Y; ++iy)
				{
					for (int32 ix = 0; ix < ItemSize.X; ++ix)
					{
						if (ShopGridMap[(y + iy) * X_COUNT + (x + ix)])
						{
							bCanFit = false; break;
						}
					}
					if (!bCanFit) break;
				}

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
		// ----------------------------------------

		// 자리(FoundPos)를 찾았다면, 인벤토리와 100% 동일한 방식으로 위젯을 생성하고 부착합니다.
		if (bFound)
		{
			// 🌟 [수정 2] this 대신 GetOwningPlayer() 사용
			UR1InventoryEntryWidget* NewEntry = CreateWidget<UR1InventoryEntryWidget>(GetOwningPlayer(), EntryWidgetClass);
			if (NewEntry)
			{
				// 🌟 [수정 3] .Add()가 아니라 인벤토리처럼 정확한 인덱스에 할당!
				int32 SlotIndex = FoundPos.Y * X_COUNT + FoundPos.X;
				EntryWidgets[SlotIndex] = NewEntry;

				UCanvasPanelSlot* CanvasSlot = CanvasPanel_Entries->AddChildToCanvas(NewEntry);
				if (CanvasSlot)
				{
					// 인벤토리의 OnInventoryEntryChanged 로직과 완벽하게 동일!
					CanvasSlot->SetAutoSize(true);

					// 🌟 [수정 4] SlotPixelSize 변수 대신, 인벤토리와 동일하게 하드코딩 50을 곱함!
					CanvasSlot->SetPosition(FVector2D(FoundPos.X * 50.0f, FoundPos.Y * 50.0f));
				}

				NewEntry->Init(this, ShopItem, ShopItem->ItemCount);
			}
		}
	}
}
