


#include "Item/R1EquipmentSlotWidget.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Item/R1DragDropOperation.h"
#include "Item/R1ItemInstance.h"
#include "Item/R1InventorySubsystem.h"
#include "Item/R1ItemDragWidget.h"
#include "R1Define.h"
#include "Blueprint/WidgetBlueprintLibrary.h"



void UR1EquipmentSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (SizeBox_Root)
	{
		FVector2D UnitSlotSize = FVector2D(Item::UnitInventorySlotSize);	
		FIntPoint SlotSize = FIntPoint(1, 1);

		// 💡 부위에 따라 기본 빈 칸의 크기를 결정합니다!
		switch (EquipmentSlotType)
		{
		case ER1EquipmentSlot::Weapon:
		case ER1EquipmentSlot::Armor:
			SlotSize = FIntPoint(2, 3); // 무기, 갑옷은 2x3
			break;
		case ER1EquipmentSlot::Helmet:
		case ER1EquipmentSlot::Boots:
		case ER1EquipmentSlot::Glove:
			SlotSize = FIntPoint(2, 2); // 투구, 신발은 2x2
			break;
		case ER1EquipmentSlot::Ring:
			SlotSize = FIntPoint(1, 1); // 반지는 1x1
			break;
		}

		SizeBox_Root->SetWidthOverride(SlotSize.X * UnitSlotSize.X);
		SizeBox_Root->SetHeightOverride(SlotSize.Y * UnitSlotSize.Y);
	}
}

bool UR1EquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UR1DragDropOperation* DragDropOp = Cast<UR1DragDropOperation>(InOperation);
	if (!DragDropOp || !DragDropOp->ItemInstance) return false;

	if (DragDropOp->ItemInstance->GetEquipSlot() != EquipmentSlotType)
	{
		UE_LOG(LogTemp, Warning, TEXT("착용할 수 없는 부위입니다!"));
		return false;
	}

	UR1InventorySubsystem* InventorySubsystem = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (InventorySubsystem)
	{
		// (기존 스왑 및 장착 로직 유지)
		UR1ItemInstance* OldEquippedItem = EquippedItem;
		InventorySubsystem->RemoveItemFromGrid(DragDropOp->ItemInstance, DragDropOp->FromItemSlotPos);

		if (OldEquippedItem != nullptr)
		{
			InventorySubsystem->AddItemToGrid(OldEquippedItem, DragDropOp->FromItemSlotPos);
		}

		InventorySubsystem->EquipItem(DragDropOp->ItemInstance);

		return true;
	}

	return false;
}

FReply UR1EquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	// 장착된 아이템이 있을 때만 드래그 시작
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && EquippedItem != nullptr)
	{
		Reply.DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	return Reply;
}

void UR1EquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!EquippedItem || !DragWidgetClass) return;

	// 드래그 잔상 위젯 생성
	UR1ItemDragWidget* DragWidget = CreateWidget<UR1ItemDragWidget>(GetOwningPlayer(), DragWidgetClass);
	const FVector2D UnitSlotSize = Item::UnitInventorySlotSize;
	FVector2D EntityWidgetSize = FVector2D(EquippedItem->GetItemSize().X * UnitSlotSize.X, EquippedItem->GetItemSize().Y * UnitSlotSize.Y);

	DragWidget->Init(EntityWidgetSize, EquippedItem->GetItemIcon(), 1);

	// 드래그 정보(Operation) 세팅
	UR1DragDropOperation* DragDrop = NewObject<UR1DragDropOperation>();
	DragDrop->DefaultDragVisual = DragWidget;
	DragDrop->Pivot = EDragPivot::MouseDown;
	DragDrop->ItemInstance = EquippedItem;
	DragDrop->FromEquipmentSlot = EquipmentSlotType;

	OutOperation = DragDrop;

	// 드래그 중에는 원래 장비창의 아이콘을 반투명하게 만듦
	Image_ItemIcon->SetRenderOpacity(0.5f);
}

void UR1EquipmentSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	if (EquippedItem)
	{
		Image_ItemIcon->SetRenderOpacity(1.0f);
	}
}

void UR1EquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UR1InventorySubsystem* InventorySubsystem = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (InventorySubsystem)
	{
		// 1. 인벤토리/장비 데이터가 변할 때마다 내 RefreshSlotUI를 실행하라고 연결!
		InventorySubsystem->OnInventoryUpdated.AddDynamic(this, &UR1EquipmentSlotWidget::RefreshSlotUI);
	}

	// 2. UI가 처음 생성될 때, 이미 장착된 템이 있는지 확인하고 그리기
	RefreshSlotUI();
}

void UR1EquipmentSlotWidget::RefreshSlotUI()
{
	UR1InventorySubsystem* InventorySubsystem = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (!InventorySubsystem) return;

	// 3. 서브시스템에서 내 부위(EquipmentSlotType)에 장착된 아이템을 가져옴
	UR1ItemInstance* LocalEquippedItem = InventorySubsystem->GetEquippedItem(EquipmentSlotType);

	if (LocalEquippedItem)
	{
		EquippedItem = LocalEquippedItem;

		// 💡 핵심: 구조체가 변경되었으므로 GetItemIcon()을 사용해야 합니다!
		if (UTexture2D* Icon = EquippedItem->GetItemIcon())
		{
			Image_ItemIcon->SetBrushFromTexture(Icon);
			Image_ItemIcon->SetRenderOpacity(1.0f);
			Image_ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
	else
	{
		// 4. 장착된 아이템이 없다면 아이콘 숨기기 (투명하게 만들고 터치 무시)
		EquippedItem = nullptr;
		Image_ItemIcon->SetRenderOpacity(0.0f);
		Image_ItemIcon->SetVisibility(ESlateVisibility::Hidden);
	}
}
