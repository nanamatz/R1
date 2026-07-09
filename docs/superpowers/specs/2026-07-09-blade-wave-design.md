# Blade Wave — Design Spec (2026-07-09)

Charged, piercing sword-beam skill for the player. Hold the skill key to charge,
release to fire a single projectile toward the mouse cursor that pierces every
monster in its path.

## Decisions (settled during brainstorming)

| Topic | Decision |
|---|---|
| Grant / key binding | Granted by weapon item DataAsset like existing skills; auto-assigns to first free slot (Q→W→E→R). No preferred-slot mechanism — the skill works from whichever slot it lands in. |
| Input model | Hold-to-charge, release-to-fire. |
| Charge bounds | 0.5s minimum, 3s maximum; at max the character **holds** the charged loop until release (no auto-fire). |
| Early release (<0.5s) | Free cancel: montage stops, no mana, no cooldown. |
| What scales with charge | Damage (lerp 1.0×→2.5× of table damage, tunable) and beam size (collision radius + visual scale, 1.0×→2.0×, tunable). Charge ratio = (elapsed − 0.5) / (3.0 − 0.5), clamped 0..1. |
| Character during charge | Rooted via Casting state (movement input already blocked while casting). Snaps to face the cursor's world position at release; fires in that direction. |
| Interruption | **Uninterruptible** (hyper-armor): HitReact is blocked while charging/firing; damage is still taken. Death/unequip still cancel cleanly. |
| Cost / cooldown | Mana checked at press (can't start without enough); deducted when the beam fires. Cooldown starts on fire. Cancelled charges cost nothing. |
| Projectile | Single beam, fixed speed, pierces all monsters, damages each exactly once, expires after covering the skill's `Range`. |

## 1. Input & release-event plumbing (slot-agnostic)

- `AR1PlayerController` adds `ETriggerEvent::Completed` + `Canceled` bindings for
  all four skill input actions. Press keeps calling `ExecuteSkillSlot(Slot)`;
  release calls new `ReleaseSkillSlot(Slot)`.
- `ReleaseSkillSlot` sends a gameplay event to the **character's ASC** (① in
  CLAUDE.md's ASC structure) with a slot-specific tag.
- New gameplay tags in `R1GameplayTags`:
  - `Event.Skill.Release` (parent) + `Event.Skill.Release.Q/W/E/R`
  - `Event.Montage.BladeWave` (fire anim-notify)
  - `Character.State.Charging` (hyper-armor marker)
- New lookup `UR1EquipmentManagerComponent::GetSlotForHandle(FGameplayAbilitySpecHandle) const`
  over the existing `SkillSlotsMap`, so an active ability can learn which slot it
  occupies and listen for exactly that slot's release tag.
- Tap-style skills ignore release events entirely. Any future hold-type skill
  reuses this plumbing unchanged.

## 2. Ability — `UR1GameplayAbility_BladeWave` (C++, extends `UR1GameplayAbility`)

`SkillID = "BladeWave"` → `FSkillDataRow` lookup via GameInstance (row already
has `Damage` / `ManaCost` / `Cooldown` / `Range`).

- **Activate (press):** verify mana ≥ ManaCost (read-only check against
  `UPlayerAttributeSet` on the PlayerState ASC ②) → enter Casting → play
  `AM_BladeWave` charge section (via the existing
  `PlayAttackMontageAndWaitForEvent` pattern) → record charge start time →
  start `WaitGameplayEvent` tasks for (a) this slot's release tag and (b)
  `Event.Montage.BladeWave`.
- **On release event:** elapsed < 0.5s → stop montage, `EndAbility` (cancel
  path, restores state). Otherwise compute charge ratio, cache damage/size
  scales, rotate pawn to `GetHitResultUnderCursor` (existing Attack trace
  channel `ECC_GameTraceChannel2`, XY-plane direction), cache the fire
  direction, `MontageJumpToSection("Fire")`.
- **On fire notify (`OnAttackEventReceived`):** spawn projectile facing the
  cached direction; build damage spec (`DamageEffect` GE using
  `UR1DamageExecutionCalc`, `SetByCaller Data.Skill.Magnitude = TableDamage ×
  DamageScale`); hand spec + size scale to the projectile; apply mana cost
  (`Data.Skill.Cost`, following `R1GameplayAbility_JumpAttack`) and cooldown.
- **End:** montage end/interrupt → `OnMontageEnded` → `EndAbility`, character
  state restored (mirrors the existing EndAbility-resets-state behavior).
- **Hyper-armor:** ability's `ActivationOwnedTags` includes
  `Character.State.Charging`; `UR1GameplayAbility_HitReact` adds that tag to its
  `ActivationBlockedTags` (C++ constructor).

## 3. Projectile — `AR1BladeWaveProjectile` (extends `AR1Projectile`)

Base class untouched (monsters keep using it). Subclass overrides `OnOverlap`:

- Never destroys on pawn overlap: applies the damage spec to the target's ASC,
  records the actor in a hit-once `TSet` (`UPROPERTY` for GC safety), keeps
  flying.
- Skips the instigator and anything without an ASC (shop NPCs unaffected).
- `SetChargeScale(float)`: scales sphere radius + visual scale.
- Ability sets speed and lifespan = `Range ÷ Speed`.
- Collision reuses the existing `MonsterProjectile` profile behavior (overlap
  pawns, ignore world geometry — same as current projectiles); the instigator
  check prevents self-hits.

## 4. Content / editor work (done by the user in-editor)

1. `AM_BladeWave`: three sections — `Charge` (wind-up) → `ChargeLoop`
   (looping while held) → `Fire`; gameplay-event anim notify
   (`Event.Montage.BladeWave`) at the swing frame of `Fire`.
2. `BP_BladeWaveProjectile`: BP of the new C++ projectile with beam VFX.
3. `GA_BladeWave`: BP of the ability — assign montage, projectile class,
   damage GE, event tags, scale ranges. `SkillType = Active` so it
   auto-assigns to a slot.
4. `DT_SkillData`: add `BladeWave` row (Damage / ManaCost / Cooldown / Range).
5. Weapon DataAsset: add `GA_BladeWave` to `GrantedAbilities`.

## 5. Edge cases & test plan

- Releasing a *different* skill key during charge → ignored (slot-tagged events).
- Release with no active charge → no listener, no-op.
- Death or weapon-unequip mid-charge → existing `CancelAbilityHandle` →
  `EndAbility` flow restores state; no mana/cooldown charged.
- UI toggles (inventory/menu) while holding the key: verify the release event
  isn't swallowed by input-mode changes — explicit test case.
- Hyper-armor: get hit during charge → no HitReact, charge continues; DoT /
  damage still applies.
- Beam damages each monster exactly once, passes through the whole pack,
  expires at `Range`.
- Tap-style skills in other slots behave exactly as before.

## Open item (verify during planning — not a design blocker)

- Exactly how existing player skills apply cooldown from
  `FSkillDataRow.Cooldown` (check `R1GameplayAbility_JumpAttack` /
  `R1GameplayAbility_BossAttackBase`); Blade Wave copies that pattern.
