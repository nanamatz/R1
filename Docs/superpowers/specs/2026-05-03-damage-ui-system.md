# Damage UI System Design Specification

## 1. Overview
The Damage UI System provides visual feedback for damage, healing, and critical hits by displaying floating text above characters in the 3D world. It uses the Gameplay Ability System (GAS) for event triggering and a World Subsystem for performant widget management and pooling.

## 2. Data Structures

### `EDamageType` (Enum)
Defines the category of damage for styling purposes.
- `Normal`: Standard damage (e.g., White text).
- `Critical`: High-impact damage (e.g., Yellow/Red text, larger scale).
- `Heal`: Health restoration (e.g., Green text).

### `FDamageInfo` (Struct)
Payload for damage events.
- `float DamageAmount`: The value to display.
- `FVector TargetLocation`: The 3D world position where the text should appear.
- `EDamageType DamageType`: The category of the damage.

## 3. Architecture

### `UR1DamageUISubsystem` (UWorldSubsystem)
The central manager for damage UI.
- **Responsibilities:**
    - Maintain a pool of `UR1DamageTextWidget` instances to avoid frequent allocation/deallocation.
    - Provide a public API: `void ShowDamageText(const FDamageInfo& DamageInfo)`.
    - Project world coordinates to screen coordinates if necessary (though often handled by adding to the viewport with a world-to-screen projection).
- **Pooling Logic:**
    - If an idle widget exists in the pool, reuse it.
    - Otherwise, spawn a new instance (up to a reasonable limit).
    - Widgets return themselves to the pool after their animation finishes.

### `UR1DamageTextWidget` (UUserWidget)
The base C++ class for the Damage Text Blueprint (`WBP_DamageText`).
- **Properties:**
    - `UCommonTextBlock* DamageText`: The text component to display the number.
    - `UWidgetAnimation* FloatAnim`: The animation for floating/fading.
- **Functions:**
    - `void SetDamageInfo(const FDamageInfo& Info)`: Updates text and triggers style/animation based on type.
    - `void ReturnToPool()`: Calls back to the subsystem to mark itself as idle.

## 4. Integration with GAS

### `UR1AttributeSet::PostGameplayEffectExecute`
- Intercepts health changes.
- If health decreases, it packages the magnitude and target location into `FDamageInfo`.
- Calls `UR1DamageUISubsystem::ShowDamageText`.

## 5. Success Criteria
- Damage numbers appear above characters when they take damage.
- Numbers are styled differently for Normal, Critical, and Heal (future-proofed).
- Performance is maintained during high-frequency hits through widget pooling.
- Text fades and moves upward smoothly.
