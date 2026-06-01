# HPTextUI Design

**Date:** 2026-06-01
**Status:** Approved
**Engine:** Unreal Engine 5.3 / C++ (GAS)

## Goal

When the mouse cursor hovers over the HP Orb widget, display a text overlay reading
`Health / MaxHealth` (e.g. `85 / 100`). The text live-updates whenever the player's
Health or MaxHealth attribute changes. The overlay is hidden when not hovering.

## Decisions (from brainstorming)

- **Structure:** HPTextUI is a child widget embedded inside `WBP_HpOrb`. The orb widget
  toggles its visibility on mouse enter/leave. Self-contained; no HUD changes.
- **Text format:** Integer `"85 / 100"` (both values floored to whole numbers, matching the
  `FMath::FloorToInt` formatting used in `R1CharacterStatUI`).
- **Update source:** Bind to the character ASC's `GetHealthAttribute` and `GetMaxHealthAttribute`
  change delegates (the `R1CharacterStatUI` pattern). Does not modify the existing
  `OnHpChanged` delegate signature, which `R1HpOrbWidget` and `R1MonsterInfoSceneWidget` depend on.

## Components

### 1. New widget class `UR1HpTextUI`

Location: `Source/R1/UI/PlayerInfo/R1HpTextUI.h` / `.cpp` (alongside `R1HpOrbWidget`).

- `UPROPERTY(meta=(BindWidget)) TObjectPtr<class UTextBlock> Text_Hp;`
  — the text element; must be named `Text_Hp` in the WBP.
- `virtual void NativeConstruct() override;`
  - Get owning pawn, cast to `AR1Player`, get `GetAbilitySystemComponent()`.
  - Bind `OnHpAttributeChanged` to:
    - `ASC->GetGameplayAttributeValueChangeDelegate(UR1AttributeSet::GetHealthAttribute()).AddUObject(this, &UR1HpTextUI::OnHpAttributeChanged)`
    - `ASC->GetGameplayAttributeValueChangeDelegate(UR1AttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UR1HpTextUI::OnHpAttributeChanged)`
  - Call `RefreshText()` once for the initial value.
  - Log an error if the player/ASC is not found (consistent with `R1HpOrbWidget`).
- `virtual void NativeDestruct() override;`
  - Remove delegate bindings on both attributes (`...RemoveAll(this)`), guarded by a valid ASC.
- `void OnHpAttributeChanged(const FOnAttributeChangeData& Data);` — calls `RefreshText()`.
- `void RefreshText();`
  - Reads `ASC->GetNumericAttribute(UR1AttributeSet::GetHealthAttribute())` and
    `...GetMaxHealthAttribute()`.
  - Formats as `FText::Format(NSLOCTEXT(...), Health, MaxHealth)` with both as
    `FMath::FloorToInt(...)` integers → `"{0} / {1}"`.
  - Sets `Text_Hp->SetText(...)` if `Text_Hp` is valid.

### 2. Modify `UR1HpOrbWidget` (header change → full VS2022 build, not Live Coding)

- Add `UPROPERTY(meta=(BindWidget)) TObjectPtr<class UR1HpTextUI> HpTextUI;`
  — the embedded child instance inside `WBP_HpOrb`.
- Override `virtual void NativeOnMouseEnter(const FGeometry&, const FPointerEvent&) override;`
  → set `HpTextUI` visibility to `ESlateVisibility::Visible`.
- Override `virtual void NativeOnMouseLeave(const FPointerEvent&) override;`
  → set `HpTextUI` visibility to `ESlateVisibility::Collapsed`.
- In `NativeConstruct`, default `HpTextUI` to `ESlateVisibility::Collapsed`.
- All visibility access guarded by a valid `HpTextUI` pointer.

## Editor setup (manual, after the build)

- Create `WBP_HpText` reparented to `UR1HpTextUI`, containing a `TextBlock` named exactly `Text_Hp`.
- In `WBP_HpOrb`: add a `UR1HpTextUI` child widget named exactly `HpTextUI` (matches the BindWidget).
- Set the HP Orb widget's **Visibility to `Visible`** (hit-testable) so `NativeOnMouseEnter/Leave` fire.

## Constraints honored

- No new replication / RPC — pure local UI, consistent with the singleplayer-only policy (CLAUDE.md §5).
- AttributeSet values are only read, never written directly (CLAUDE.md §4/§7).
- `*.generated.h` remains the last include; forward declarations used in headers.
- Header changes require a full VS2022 build (CLAUDE.md §2 Live Coding caveat).

## Testing / verification

- Compile via the editor build target.
- In-editor: hover the orb → text appears showing current `HP / MaxHP`; move away → hides.
- Take damage / heal → text updates live. Change MaxHealth (e.g. via a stat upgrade) → max updates.
