# Three New Bosses (2F / 3F / 5F) — Design Spec (2026-08-14)

Three new floor bosses built almost entirely from the ability classes that already
ship in `Source/R1/AbilitySystem/Abilities/`. Adds a shared HP-threshold phase
system to `AR1Boss` so all bosses (Baby included) can change behaviour mid-fight.

`Content/Data/RoomDatas/Boss/` already holds `DA_Boss_1F/2F/3F/5F`. 1F is Baby
(`BP_Baby`, `ABossBaby`). This spec fills 2F, 3F, 5F. 4F has no boss room and is
out of scope.

## Decisions (settled during brainstorming)

| Topic | Decision |
|---|---|
| Floor mapping | 2F Zoner, 3F Bruiser, 5F Summoner. New skeletal meshes + montages are being authored separately; this spec is art-agnostic and leaves montage/socket assignment to the Blueprint pass. |
| Reuse depth | New C++ allowed where no existing class covers the pattern. Existing player-only / Baby-only abilities get generalized rather than duplicated. |
| Phases | Shared HP-threshold phase system on `AR1Boss`, driven by a `TArray<FBossPhase>`. Empty array = today's behaviour, so Baby is unaffected. |
| Boss AI | All three reuse `BT_Baby`'s structure and `BB_Boss`. Variety comes from the phase lists and per-skill cooldowns, not from new Behavior Trees. |
| Skill weighting | Not implemented. Per-skill cooldown gating provides pattern control; add weights only if tuning proves it insufficient. |
| Telegraph shapes | Circle and Rectangle only. `ER1TelegraphShape::Cone` exists in the enum but has no decal material; avoided. |
| Boss names | `ABossWarden` / `ABossRavager` / `ABossHierarch` are placeholders until meshes land. Rename before the Blueprint pass if the art has different names. |

---

## Part 1 — Shared framework changes

Four of these are prerequisites, not polish: without 1A and 1B every boss picks a
random skill every 0.5s and the designed rotations do not exist.

### 1A. Boss skill cooldowns are never applied

`UR1GameplayAbility_BossAttackBase::OnAvatarSet` caches `CachedCooldown` from the
`FSkillDataRow`, but nothing ever applies it — there is no `ApplyCooldown`
override. Every boss skill is permanently off cooldown.

`UR1GameplayAbility_BladeWave` already solves exactly this for the player
(`R1GameplayAbility_BladeWave.cpp:279`). Copy it.

**Change** (`R1GameplayAbility_BossAttackBase.h/.cpp`): override `ApplyCooldown`
with BladeWave's body verbatim. No new `UPROPERTY` — `CooldownGameplayEffectClass`
is a built-in `UGameplayAbility` member, assignable from any ability Blueprint.
No `CheckCooldown` override — the stock tag-based check works once the GE grants
a tag.

**No new gameplay tags.** `Data.Skill.Cooldown` already exists
(`R1GameplayTags.h:68`).

**New asset:** `GE_BossSkillCooldown` — Duration policy `HasDuration`, duration
magnitude `SetByCaller` on `Data.Skill.Cooldown`. Each ability Blueprint gets its
own child of this GE granting its own `Cooldown.Boss.<SkillID>` tag (plain asset
tags, no C++ declaration needed). One shared tag across all boss skills would put
the entire kit on a single global cooldown, so per-skill tags are required, not
optional.

### 1B. Skill picker must respect cooldowns

`UBTService_PrepareSkill::TickNode` (`Source/R1/AI/BTService_PrepareSkill.cpp:63`)
picks uniformly at random from the whole list every 0.5s:

```cpp
int32 RandomIndex = FMath::RandRange(0, AbilityList.Num() - 1);
```

**Change:** build a candidate array first, keeping only abilities whose granted
`FGameplayAbilitySpec` passes `Spec.Ability->CanActivateAbility(Spec.Handle, ActorInfo)`
(this covers cooldown, cost, and blocking tags in one call). Random-pick from the
candidates. If the candidate list is empty, fall through to the unfiltered
in-range default list so the boss never stalls with no selection.

The distance/angle branch that chooses `GetDefaultSkillList()` vs
`GetAdditionalSkillList()` is unchanged.

### 1C. Phase system on `AR1Boss`

```cpp
USTRUCT(BlueprintType)
struct FBossPhase
{
    GENERATED_BODY()

    // Enter this phase when Health/MaxHealth drops to or below this value.
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HealthRatioThreshold = 0.5f;

    // Replaces the boss's in-range skill list on entry. Empty = keep current.
    UPROPERTY(EditAnywhere)
    TArray<TSubclassOf<UGameplayAbility>> DefaultSkills;

    // Replaces the out-of-range skill list on entry. Empty = keep current.
    UPROPERTY(EditAnywhere)
    TArray<TSubclassOf<UGameplayAbility>> AdditionalSkills;

    // Infinite GE applied on entry (AttackSpeed / MoveSpeed multipliers). Never removed.
    UPROPERTY(EditAnywhere)
    TSubclassOf<UGameplayEffect> EnrageEffect;

    // Optional transition animation played on entry.
    UPROPERTY(EditAnywhere)
    TObjectPtr<UAnimMontage> TransitionMontage;
};
```

On `AR1Boss`:

- `UPROPERTY(EditAnywhere, Category = "Phases") TArray<FBossPhase> Phases;`
  — authored in descending threshold order (0.55, 0.20). Validated in
  `BeginPlay` with `ensureMsgf` if out of order.
- `int32 CurrentPhaseIndex = INDEX_NONE;`
- Runtime lists `ActiveDefaultSkills` / `ActiveAdditionalSkills`, initialized in
  `BeginPlay` from `DefaultSkillAbilities` / `AdditionalSkillAbilities`.
  `GetDefaultSkillList()` / `GetAdditionalSkillList()` return the active lists.
- Override `OnHealthChanged(float Ratio, bool bIsDamage)` — already `virtual`,
  already called from `UR1AttributeSet::PostGameplayEffectExecute`
  (`R1AttributeSet.cpp:95`). No new plumbing, no tick.

**Entry logic** (`AdvanceToPhase`), on damage only:

1. While `CurrentPhaseIndex + 1 < Phases.Num()` and
   `Ratio <= Phases[CurrentPhaseIndex + 1].HealthRatioThreshold`, advance one
   phase. The loop (rather than a single check) handles a burst that crosses two
   thresholds at once.
2. For each phase entered: grant every ability in `DefaultSkills` +
   `AdditionalSkills` to the ASC via `UR1AbilitySystemComponent::AddCharacterAbilities`
   (idempotent for already-granted classes — verify before relying on this;
   if it double-grants, guard with a `TSet<UClass*>` of already-granted classes).
3. Replace the active lists with the non-empty ones from the phase.
4. Apply `EnrageEffect` to self. Successive phases stack their own GEs; earlier
   enrage effects are not removed.
5. Play `TransitionMontage` if set.
6. Set blackboard key `Phase` (new int key on `BB_Boss`) to the new index —
   available for designers who want phase-gated BT branches later. Not consumed
   by this spec.

`AddCharacterAbility()` continues to grant `DefaultSkillAbilities` +
`AdditionalSkillAbilities` at spawn; phase abilities are granted lazily on entry.

### 1D. Projectile fields belong on `AR1Monster`

`UR1GameplayAbility_RangedAttack::OnAttackEventReceived` (`R1GameplayAbility_RangedAttack.cpp:8`)
hard-casts `AR1RangerMonster` and returns silently for anything else. Same for
`UR1GameplayAbility_WaveAttack::OnAttackEventReceived` (`R1GameplayAbility_WaveAttack.cpp:14`),
which casts `ABossBaby` purely to read `MuzzleSocketName`.

**Change:** move `ProjectileClass` and `MuzzleSocketName` from `AR1RangerMonster`
(and `ABossBaby`) up to `AR1Monster`. Change both abilities to cast `AR1Monster`.
Ranger and Baby keep working through inheritance; every boss gains projectiles
and rectangle attacks. Delete the now-duplicate declarations in the subclasses.

### 1E. `WaveAttack` damage is gated on its VFX

In `R1GameplayAbility_WaveAttack.cpp:57`, the entire overlap-and-damage block is
nested inside `if (WaveEffect)`. A Blueprint child with no particle assigned
deals no damage and logs nothing.

**Change:** hoist the overlap/damage block out of the `if (WaveEffect)` branch.
Keep only the `SpawnEmitterAtLocation` + `SetVectorParameter("BeamTarget")` calls
inside it.

### 1F. `UR1GameplayAbility_BossLeap` (new)

`UR1GameplayAbility_JumpAttack` is unusable for AI: `CanActivateAbility`
(`R1GameplayAbility_JumpAttack.cpp:37`) fails any non-`AR1Player` avatar, and the
target comes from `AR1PlayerController::GetHighlightActor()`. Not worth
retrofitting — the player version also consumes player mana and player input state.

**New class** `UR1GameplayAbility_BossLeap : public UR1GameplayAbility_BossAttackBase`:

- Target = the AI controller's blackboard `TargetActor` (same key `BB_Boss` uses),
  read once on activation.
- Root-motion dash via `UAbilityTask_ApplyRootMotionMoveToActorForce` — the same
  construction as `JumpAttack.cpp:154`, minus the player plumbing. Targeting the
  actor (not a frozen location) means the leap tracks a moving player, which is
  what makes it a threat.
- On root-motion finish (`bReachedDestination` / `bTimedOut` both land): sphere
  overlap at the landing point with radius `TelegraphData->TelegraphSize.X`,
  filtered to `AR1Player`, apply `DamageEffect` with
  `Data.Skill.Magnitude` = `CachedDamage`, fire `GameplayCue.Weapon.Impact` —
  identical to `GroundAttack::OnAttackEventReceived`.
- Telegraph actor is spawned by the base class at the *boss's* location on
  activation, which is wrong for a leap. Override the spawn to place it at the
  target's location at activation time. It will not perfectly match a tracking
  landing point; that mismatch is acceptable telegraph slop and the alternative
  (a telegraph actor that follows the target) is out of scope.
- `EndAbility` on both the root-motion-finished and interrupted paths.

Estimated ~120 lines, most lifted from `JumpAttack.cpp`.

### 1G. `UR1GameplayAbility_SummonAdds` (new)

`AR1MonsterSpawner` is a level-placed actor bound to an `ADungeonManager` and
driven by `FMonsterSpawnPreset` arrays — wrong shape for an ability. Summoning
gets its own class.

**New class** `UR1GameplayAbility_SummonAdds : public UR1GameplayAbility_BossAttackBase`:

- `TArray<TSubclassOf<AR1Monster>> AddClasses` — one entry picked at random per spawn.
- `int32 SpawnCount = 2` — how many to spawn per activation.
- `int32 AliveCap = 4` — refuse activation (`CanActivateAbility` returns false)
  when live adds ≥ cap, so the picker in 1B skips it cleanly.
- `float SpawnRingRadius = 400.f` — spawn positions evenly distributed on a ring
  around the avatar, each projected to the navmesh via
  `UNavigationSystemV1::ProjectPointToNavigation`. Skip any point that fails to
  project rather than spawning an add in geometry.
- `TArray<TWeakObjectPtr<AR1Monster>> SpawnedAdds` — pruned of stale/dead entries
  each activation to compute the live count.
- Spawned adds get `InitializeWithManager(Boss->OwningDungeonManager)` so their
  death bookkeeping matches spawner-placed monsters.
- Damage-free: `OnAttackEventReceived` does the spawning at the montage notify.

Estimated ~90 lines.

> Adds spawned by the boss are ordinary monsters and grant their normal XP and
> gold. If that proves exploitable during tuning, suppress rewards with a flag on
> the ability rather than a new monster class.

### 1H. `ChainLightning` needs a self-cast path

`UR1GameplayAbility_ChainLightning::ActivateAbility`
(`R1GameplayAbility_ChainLightning.cpp:29`) reads its first target from
`TriggerEventData->Target` — it is an on-hit-triggered player ability with no
self-targeting entry point. The 5F phase-2 use requires a fallback.

**Change:** when `TriggerEventData` is null or its `Target` is invalid, resolve
the initial target from the avatar's AI controller blackboard `TargetActor`. Bail
out with a warning log if neither is available. ~8 lines, no change to the bounce
logic. The `AR1Player`-specific branch at line 107 is VFX-only and already
guarded, so it no-ops for a boss avatar.

---

## Part 2 — The three bosses

Each is a thin `AR1Boss` child in `Source/R1/Character/Boss/`, following
`BossBaby.h`. All three use `BB_Boss` and a copy of `BT_Baby`'s structure.

All damage / cooldown / range numbers below live in `DT_BossSkillData` rows
keyed by `SkillID`; the values in the CD column are the intended starting
cooldowns, not hardcoded constants.

### 2F — Zoner (`ABossWarden`)

Teaches spacing. Punishes both standing still and hugging.

| List | Ability asset | `SkillID` | Reused class | Telegraph | CD |
|---|---|---|---|---|---|
| Default (in range) | `GA_WardenRepulse` | `WardenRepulse` | `GroundAttack` | Circle r=350 | 6s |
| Additional | `GA_WardenVolley` | `WardenVolley` | `RangedAttack` | — | 4s |
| Additional | `GA_WardenBeam` | `WardenBeam` | `WaveAttack` | Rect 1400×250, 1.5s | 9s |

`GA_WardenVolley` uses the inherited `ComboSections` on `UR1GameplayAbility_Attack`
to fire a 3-shot burst from one montage.

**Phase 2 @ 55% HP**
- `EnrageEffect`: `GE_WardenEnrage` — Infinite, `AttackSpeed ×1.25`
- `AdditionalSkills`: `GA_WardenVolley`, `GA_WardenBeam`, `GA_WardenBeamWide`
  (`WaveAttack`, Rect 1400×500, telegraph 1.0s, `SkillID` `WardenBeamWide`, CD 9s)
- `DefaultSkills`: empty (in-range behaviour unchanged)

### 3F — Bruiser (`ABossRavager`)

Teaches punish windows. Constant pressure, no safe distance.

| List | Ability asset | `SkillID` | Reused class | Telegraph | CD |
|---|---|---|---|---|---|
| Default | `GA_RavagerCombo` | `RavagerCombo` | `MonsterComboAttack` (3 sections) | — | 2s |
| Additional | `GA_RavagerLeap` | `RavagerLeap` | **`BossLeap`** (1F) | Circle r=300 at landing | 8s |
| Additional | `GA_RavagerCharge` | `RavagerCharge` | `WaveAttack` | Rect 1000×200, 1.0s | 7s |

**Phase 2 @ 50% HP**
- `EnrageEffect`: `GE_RavagerEnrage` — Infinite, `MoveSpeed ×1.2`, `AttackSpeed ×1.3`
- `DefaultSkills`: `GA_RavagerComboRage` (`MonsterComboAttack`, 4 sections,
  `SkillID` `RavagerComboRage`, CD 2s)
- `AdditionalSkills`: `GA_RavagerLeap`, `GA_RavagerCharge`, `GA_RavagerShockwave`
  (`GroundAttack` r=600, telegraph 1.2s, `SkillID` `RavagerShockwave`, CD 10s)

**Phase 3 @ 20% HP**
- `EnrageEffect`: `GE_RavagerFrenzy` — Infinite, `AttackSpeed ×1.2` (stacks on
  top of the phase-2 GE, which is not removed)
- `AdditionalSkills`: same three, but `GA_RavagerLeap` is swapped for
  `GA_RavagerLeapRage` (`SkillID` `RavagerLeapRage`, CD 4s)
- `DefaultSkills`: empty (keeps the 4-section combo)

### 5F — Summoner (`ABossHierarch`)

Final boss. Teaches target priority.

| List | Ability asset | `SkillID` | Reused class | Telegraph | CD |
|---|---|---|---|---|---|
| Default | `GA_HierarchStaff` | `HierarchStaff` | `MonsterMeeleAttack` | — | 3s |
| Additional | `GA_HierarchSummon` | `HierarchSummon` | **`SummonAdds`** (1G) | — | 20s |
| Additional | `GA_HierarchBolt` | `HierarchBolt` | `RangedAttack` | — | 5s |
| Additional | `GA_HierarchNova` | `HierarchNova` | `GroundAttack` r=900 | Circle, 2.5s | 15s |

`GA_HierarchSummon` phase-1 config: `SpawnCount = 2`, `AliveCap = 4`,
`AddClasses = [BP_Minion, BP_TaintedMinion]`.

**Phase 2 @ 66% HP**
- `EnrageEffect`: `GE_HierarchEnrage` — Infinite, `AttackSpeed ×1.15`
- `AdditionalSkills`: the three above plus `GA_HierarchChain`
  (`ChainLightning`, `MaxBounces = 4`, `SkillID` `HierarchChain`, CD 12s)

**Phase 3 @ 33% HP**
- `EnrageEffect`: none (the fight is already dense; difficulty comes from CDs)
- `AdditionalSkills`: `GA_HierarchSummonMass` (`SpawnCount = 3`, `AliveCap = 6`,
  `SkillID` `HierarchSummonMass`, CD 12s), `GA_HierarchBolt`,
  `GA_HierarchNovaFast` (`SkillID` `HierarchNovaFast`, CD 9s), `GA_HierarchChain`

## Part 3 — Assets and data

**New `DT_BossSkillData` rows** (17): `WardenRepulse`, `WardenVolley`,
`WardenBeam`, `WardenBeamWide`, `RavagerCombo`, `RavagerComboRage`,
`RavagerLeap`, `RavagerLeapRage`, `RavagerCharge`, `RavagerShockwave`,
`HierarchStaff`, `HierarchSummon`, `HierarchSummonMass`, `HierarchBolt`,
`HierarchNova`, `HierarchNovaFast`, `HierarchChain`.

Each row carries `Damage` / `ManaCost` / `Cooldown` / `Range` per `FSkillDataRow`.
`ManaCost` stays 0 for all boss skills — the boss cost path in
`BossAttackBase::CheckCost` is a no-op stub.

**New GEs:** `GE_BossSkillCooldown` (shared), `GE_WardenEnrage`,
`GE_RavagerEnrage`, `GE_RavagerFrenzy`, `GE_HierarchEnrage`.

**New telegraph DataAssets** (`UR1TelegraphData`), one per distinct shape/size
above — roughly 9.

**New Blueprints:** `BP_Warden`, `BP_Ravager`, `BP_Hierarch`; three
`BT_*` copies of `BT_Baby`; 17 `GA_*` ability Blueprints under
`Content/Blueprints/AbilitySystem/GA/Monster/<BossName>/`; one `GA_*HitReact`
per boss (children of the existing HitReact class, matching `GA_BabyHitReact`).

**Blackboard:** `BB_Boss` gains an int key `Phase`.

**Room data:** point `DA_Boss_2F` / `DA_Boss_3F` / `DA_Boss_5F` at the new
boss Blueprints and their arena maps.

---

## Part 4 — Build and verification

Every change in Part 1 touches headers (`UPROPERTY` / `USTRUCT` additions), so
Live Coding will not pick them up. Close the editor and run a full build:

```bat
"<UE_PATH>\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "<ProjectPath>\R1.uproject" -waitmutex
```

**Verification, in order — each gates the next:**

1. **Cooldowns fire (1A).** Baby only, no new content. Set `GroundAttack`'s
   `Cooldown` row to 10s, assign `GE_BossSkillCooldown`, fight Baby. Ground
   attack must not repeat inside 10s. Before this passes, nothing else is testable.
2. **Picker filters (1B).** Same fight: with ground attack on cooldown, Baby must
   fall back to wave attack instead of standing idle. Confirm via the existing
   `BTService_PrepareSkill: Selected Ability %s` log.
3. **Phases (1C).** Give Baby a throwaway phase at 50% with an obvious enrage GE.
   Confirm single entry (not repeated every damage tick), and confirm a
   large hit that crosses two thresholds at once advances both.
4. **Generalized abilities (1D/1E).** Ranger still shoots; Baby's wave still
   damages with its particle assigned, and now also damages with it cleared.
5. Then each boss end to end: telegraph appears before damage, damage lands only
   inside the telegraph, phase transition visibly changes the rotation, and adds
   respect `AliveCap`.

No automated test harness exists for GAS abilities in this project; verification
is in-editor PIE against `Content/Maps/1F/BossMap.umap` and the new arenas.

---

## Explicitly out of scope

- Per-skill weight tables — cooldown gating covers pattern control for now.
- Boss-specific Behavior Trees beyond copies of `BT_Baby`.
- Cone telegraph shape (enum member exists, no decal material).
- Add-aggro or add-leash logic — adds use the standard monster AI.
- Shields, heal windows, or damage-immunity phases.
- 4F boss (no boss room data asset exists).
