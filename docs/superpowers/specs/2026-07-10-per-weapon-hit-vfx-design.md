# Per-Weapon Hit Impact VFX — Design

**Date**: 2026-07-10
**Branch**: `feat-various-VFX-on-Weapon-Type`
**Status**: Approved

## Goal

Play a different one-shot hit-impact VFX (Niagara) at the melee impact point depending on the equipped weapon. Each weapon's item Data Asset chooses its own VFX; hits without one (unarmed, legacy DAs, monsters) fall back to a project-wide default so every melee hit has visual feedback.

## Decisions (settled in brainstorming)

- **VFX driver**: per-item asset field on `UR1ItemAssetData` (not keyed by `WeaponType`/`ElementType`).
- **Scope**: melee only — the `GA_Attack` → `GameplayCue.Weapon.Impact` path. Projectiles (`AR1Projectile.ImpactEffect`) and skill abilities are unchanged.
- **Data shape**: single `TSoftObjectPtr<UNiagaraSystem>` field, not a tag-keyed map.
- **Fallback**: default impact VFX configured on the cue notify BP.
- **Monster hits**: included — monsters have no equipment, so they resolve to the default VFX.
- **Delivery**: Approach A — the cue notify resolves the VFX itself from the instigator's equipment (no cue-payload plumbing; `CueParams.SourceObject` stays reserved for the weapon sound).

## Current State (pre-change)

- `R1GameplayAbility_Attack.cpp:165-210`: on melee hit, line-traces source→target for impact point/normal, then executes `GameplayCue.Weapon.Impact` **only when the equipped weapon has a routed sound** (`WeaponSound && GameplayCueTag.IsValid()`). Otherwise plays `SoundToPlay` directly. `CueParams.SourceObject` = sound, `Instigator` = source character, `Location`/`Normal` = traced impact.
- `GCN_WeaponImpact` (BP, `Content/Blueprints/Audio/GCN/`): plays the sound from `SourceObject`. Audio only — no VFX today.
- `UR1ItemAssetData` already has `WeaponType`, `ElementType`, `WeaponAuraVFX`, `WeaponActorClass`, and `AudioRoutingMap` (the tag→sound pattern this design mirrors).
- `UR1EquipmentManagerComponent::GetSoundByTag()` is the existing per-slot DA lookup pattern.

## Design

### 1. Data — `UR1ItemAssetData`

Add one field next to `WeaponAuraVFX`:

```cpp
// 명중 시 피격 지점에 재생되는 원샷 임팩트 이펙트 (미지정 시 GCN의 기본 이펙트 사용)
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
TSoftObjectPtr<UNiagaraSystem> HitImpactVFX;
```

### 2. Lookup — `UR1EquipmentManagerComponent`

New accessor mirroring `GetSoundByTag`:

```cpp
UNiagaraSystem* GetHitImpactVFX(ER1EquipmentSlot EquipSlot) const;
```

Finds the equipped item DA for the slot, `LoadSynchronous()` on `HitImpactVFX`, returns `nullptr` if the slot is empty or the field is unset.

### 3. New cue notify — `UR1GameplayCueNotify_WeaponImpact`

C++ class extending `UGameplayCueNotify_Static`, located at `Source/R1/AbilitySystem/R1GameplayCueNotify_WeaponImpact.{h,cpp}` (next to `R1GameplayCueNotify_AttachedVFX`). On `Executed`:

- **Sound**: if `Parameters.SourceObject` is a `USoundBase`, play it at `Parameters.Location` (moves the current `GCN_WeaponImpact` BP logic into C++).
- **VFX**: resolve in order:
  1. `Parameters.Instigator` → cast `AR1Player` → `GetEquipmentComponent()` → `GetHitImpactVFX(ER1EquipmentSlot::Weapon)`
  2. else `DefaultHitVFX` (`UPROPERTY(EditDefaultsOnly)`, set on the BP)

  Spawn via `UNiagaraFunctionLibrary::SpawnSystemAtLocation` at `Parameters.Location`, rotation from `Parameters.Normal` (`Normal.Rotation()`).

### 4. Ability change — `R1GameplayAbility_Attack.cpp`

Loosen the cue gate so the cue fires on **every** melee hit when `GameplayCueTag.IsValid()` (the impact trace already runs there). `SourceObject` carries the weapon sound or null. The existing direct `SoundToPlay` fallback still plays when there is no weapon sound — **audio behavior is unchanged**.

Net behavior change: player weapons show their own VFX; unarmed and monster melee hits show the default VFX (previously nothing).

### 5. Editor steps (user, in-editor — .uassets are not editable from CLI)

1. Full VS2022 build with the editor closed (header changes → no Live Coding).
2. Reparent `GCN_WeaponImpact` to `UR1GameplayCueNotify_WeaponImpact`; delete the now-duplicated BP sound logic; keep its GameplayCue tag.
3. Set `DefaultHitVFX` on the BP.
4. Assign `HitImpactVFX` on weapon DAs as desired.

## Error handling

- No `AR1Player` instigator / no equipment / unset field → default VFX (never a crash, never silent-skip).
- `DefaultHitVFX` also unset → skip VFX quietly (matches today's behavior).
- Sound null → VFX still plays; sound path unchanged.

## Testing (manual, PIE)

1. Weapon with `HitImpactVFX` set → its VFX at the traced impact point, weapon sound unchanged.
2. Weapon without the field / unarmed → default VFX, fallback sound unchanged.
3. Monster hits the player → default VFX at impact.
4. Rapid combo hits → no VFX leakage/accumulation (one-shot systems auto-destroy).

## Out of scope

- Projectile impacts (`AR1Projectile.ImpactEffect` stays Cascade/per-BP).
- Skill abilities (WaveAttack etc.) — can adopt the same cue later.
- Element-based VFX variation (achievable today by assigning per-DA).
- Async preloading of `HitImpactVFX` (follows the existing `LoadSynchronous` audio pattern; revisit if hitching appears).
