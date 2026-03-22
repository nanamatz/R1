# Shop UI Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the Shop UI system, allowing players to buy items with rarity-specific visuals and sell stackable items with quantity selection via a popup.

**Architecture:** 
- **UR1ShopWidget** manages 3 `UR1ShopSlotWidget` instances.
- **UR1ShopSlotWidget** displays item info (rarity color-coded) and handles "Buy".
- **UR1SellQuantityPopup** handles "Sell with Quantity" logic for stackable items.
- **R1InventorySubsystem** provides the backend logic for transactions.
- **AR1MerchantNPC** triggers the UI.

**Tech Stack:** Unreal Engine 5 (C++), UMG.

---

### Task 1: Update R1InventorySubsystem to handle selling with quantity

**Files:**
- Modify: `Source/R1/Item/R1InventorySubsystem.h`
- Modify: `Source/R1/Item/R1InventorySubsystem.cpp`

- [ ] **Step 1: Update `SellItem` signature in `R1InventorySubsystem.h`**
  ```cpp
  // From
  void SellItem(UR1ItemInstance* Item);
  // To
  void SellItem(UR1ItemInstance* Item, int32 Quantity = 1);
  ```

- [ ] **Step 2: Update `SellItem` implementation in `R1InventorySubsystem.cpp`**
  - Calculate `SaleValue` based on `Quantity`.
  - Reduce `ItemCount` by `Quantity`.
  - If `ItemCount <= 0`, remove item from grid and list.

- [ ] **Step 3: Update `BuyItem` logic in `R1InventorySubsystem.cpp` if needed** (Ensure it handles `EItemRarity`).

- [ ] **Step 4: Commit**
  ```bash
  git add Source/R1/Item/R1InventorySubsystem.h Source/R1/Item/R1InventorySubsystem.cpp
  git commit -m "feat(inventory): update SellItem to handle quantity"
  ```

### Task 2: Implement UR1ShopSlotWidget

**Files:**
- Modify: `Source/R1/UI/Shop/R1ShopSlotWidget.h`
- Modify: `Source/R1/UI/Shop/R1ShopSlotWidget.cpp`

- [ ] **Step 1: Define UI Bindings and `SetItem` function in `R1ShopSlotWidget.h`**
  ```cpp
  // meta = (BindWidget) for Icon, Name, RarityText, RarityBackground, PriceText, BuyButton
  void SetItem(UR1ItemAssetData* InItemData);
  ```

- [ ] **Step 2: Implement `SetItem` in `R1ShopSlotWidget.cpp`**
  - Update UI based on `InItemData`.
  - Use rarity to determine `RarityText` and `RarityBackground` color.

- [ ] **Step 3: Implement "Buy" button click logic**
  - Call `R1InventorySubsystem::BuyItem()`.

- [ ] **Step 4: Commit**
  ```bash
  git add Source/R1/UI/Shop/R1ShopSlotWidget.h Source/R1/UI/Shop/R1ShopSlotWidget.cpp
  git commit -m "feat(ui): implement UR1ShopSlotWidget logic"
  ```

### Task 3: Implement UR1ShopWidget

**Files:**
- Modify: `Source/R1/UI/Shop/R1ShopWidget.h`
- Modify: `Source/R1/UI/Shop/R1ShopWidget.cpp`

- [ ] **Step 1: Define UI Bindings and `SetShopItems` in `R1ShopWidget.h`**
  - Horizontal Box for 3 slots.
  - `SetShopItems(const TArray<UR1ItemAssetData*>& Items)` function.

- [ ] **Step 2: Implement `SetShopItems` in `R1ShopWidget.cpp`**
  - Initialize the 3 `UR1ShopSlotWidget` instances with the provided items.

- [ ] **Step 3: Bind Gold Display to `R1InventorySubsystem::OnGoldChanged`**

- [ ] **Step 4: Commit**
  ```bash
  git add Source/R1/UI/Shop/R1ShopWidget.h Source/R1/UI/Shop/R1ShopWidget.cpp
  git commit -m "feat(ui): implement UR1ShopWidget logic"
  ```

### Task 4: Implement UR1SellQuantityPopup

**Files:**
- Create: `Source/R1/UI/Shop/R1SellQuantityPopup.h`
- Create: `Source/R1/UI/Shop/R1SellQuantityPopup.cpp`

- [ ] **Step 1: Define UI Bindings (Icon, Name, QuantityText, PlusButton, MinusButton, ConfirmButton, CancelButton)**

- [ ] **Step 2: Implement Quantity Adjustment Logic (+ / -)**
  - Clamp between 1 and `Item->ItemCount`.

- [ ] **Step 3: Implement Confirm/Cancel logic**
  - Confirm: Call `R1InventorySubsystem::SellItem(Item, SelectedQuantity)`.

- [ ] **Step 4: Commit**
  ```bash
  git add Source/R1/UI/Shop/R1SellQuantityPopup.h Source/R1/UI/Shop/R1SellQuantityPopup.cpp
  git commit -m "feat(ui): implement UR1SellQuantityPopup"
  ```

### Task 5: Integrate with Merchant NPC

**Files:**
- Modify: `Source/R1/Object/R1MerchantNPC.cpp` (Assuming `.h` already has the functions)

- [ ] **Step 1: Implement `OpenShop()`**
  - Create the `UR1ShopWidget`.
  - Pass `ItemsForSale` to the widget.
  - Add to viewport and set input mode.

- [ ] **Step 2: Commit**
  ```bash
  git add Source/R1/Object/R1MerchantNPC.cpp
  git commit -m "feat(merchant): implement OpenShop interaction"
  ```
