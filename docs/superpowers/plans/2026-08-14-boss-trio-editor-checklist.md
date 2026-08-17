# Boss Trio — Editor Work Checklist

Companion to `docs/superpowers/plans/2026-08-14-boss-trio.md`. All C++ is written and
committed (`0bd15cc` … `d32bcd1`); everything in this document is Unreal Editor work.

**Nothing has been run yet.** Nine behavioral changes are committed and verified only
to compile. Phase 0 exists to catch that before any new boss content is built on top.

---

## Conventions used below

- **Property names** are given as they appear in the editor Details panel, with the
  C++ name in parentheses where they differ noticeably.
- **Bold** asset names are new assets you create. `Code font` is an exact value to type.
- Each item is a checkbox. Work top to bottom — later phases depend on earlier ones.

## Seven mistakes that produce a silently do-nothing boss

Read these once before starting. Every one of these was hit for real while building
the Hierarch. Each fails with no error, no crash — and, unless noted, no log either.

0. **`BP_R1GameInstance` → `Boss Skill Data Table` not set.** Do this **first**, once,
   before any other work: set it to `DT_BossSkillData`. The GameInstance holds one
   table pointer per lookup, and the boss lookup is separate from the player one. Unset,
   every boss `SkillID` resolves to nothing and *both* Damage and Cooldown silently
   become 0 — which is why boss cooldowns "don't work" no matter what you change in the
   DataTable. Damage still lands (it comes from `BaseDamage` via
   `R1DamageExecutionCalc`), which is what makes this so hard to spot.
   *Logs a warning since `d5357b4`, so check for `BossSkillDataTable이 비어있어`.*
1. **Missing AnimNotify.** Every attack montage needs an AnimNotify sending
   `Event.Montage.Attack` at the damage frame. Without it `OnAttackEventReceived`
   never fires — the boss plays its animation and deals nothing. Combo abilities need
   **one notify per section**.
   *Logs a warning since `0cd1f22`: `montage ended without ever receiving '...'`.*

   ⚠️ **The registered tag string is misspelled** — `"Event.Monage.Attack"`, missing the
   `t` (`R1GameplayTags.cpp:19`). The C++ symbol is spelled correctly, so nothing in code
   breaks, but a notify whose tag you **typed by hand** as `Event.Montage.Attack` will
   never match. Always pick the tag from the picker. Do not "fix" the string casually:
   every existing notify in the project uses the typo'd version and would break at once.
2. **Missing Activation Blocked Tag.** A cooldown GE granting `Cooldown.Boss.X` does
   **not** by itself stop re-activation. The ability must also list `Cooldown.Boss.X`
   in **Activation Blocked Tags**. Without it the cooldown is decorative and the boss
   spams one skill.
3. **Wrong room level in the RoomData asset.** `DA_Boss_*F` must point at a gameplay
   map containing a `DungeonManager` and a `NavMeshBoundsVolume`. `PLA_Map` levels are
   art-only — pointing at one leaves the room unregistered and the navmesh gate times out.
4. **Phase with no Transition Montage gets no gate.** The whole phase-gate behaviour
   hangs off `TransitionMontage`. Leave it empty and `BeginPhaseTransition` returns
   early: no health pin at the threshold, no hyper-armor, no BT pause — health sails
   straight through the threshold and the phase swap just happens mid-swing. The skill
   lists and enrage GE still apply, so it looks *almost* right, which is what makes it
   easy to miss. Every phase that should read as a beat needs a montage assigned.
5. **Anim Blueprint has no `Slot 'DefaultSlot'` node.** The montage plays, notifies fire,
   damage lands — and nothing animates, because the pose never reaches the output. Every
   new boss ABP needs a Slot node between its state machine and the final pose, matching
   the slot named in the montage. Cost a full debugging round on the Hierarch.
6. **`Flow Abort Mode: None` on a distance-gated branch.** A branch whose condition is
   distance-based will not re-evaluate while another branch runs, so the skill only ever
   fires right after some other task happens to finish. Gate such branches on a
   **Blackboard** decorator (`CanAttack`) with `Flow Abort Mode: Lower Priority` —
   custom decorators like `IsTooClose` derive from `BTDecorator_BlackboardBase` and only
   re-check when the *key* changes, which distance never does.

---

# Phase 0 — Verify the committed C++ before building on it

Do this first, on `Content/Maps/1F/BossMap.umap`, with `LogR1` visible in the Output Log.
Everything here is throwaway and gets deleted in Phase 0.6.

## 0.1 Regression check — Task 4 moved two properties

`ProjectileClass` and `MuzzleSocketName` moved from `AR1RangerMonster` / `ABossBaby`
up to `AR1Monster`. A single base default cannot preserve both former defaults, so the
base keeps `Muzzle_Front` and Baby must be re-set by hand.

- [ ] Open **`BP_Baby`** → Details → category **Combat** → **Muzzle Socket Name**.
      If it reads `Muzzle_Front`, change it to `Muzzle_02`. Save.
- [ ] Open **`BP_Ranger`** → **Muzzle Socket Name** should read `Muzzle_Front`, and
      **Projectile Class** should still point at its projectile. Re-set if blank.
- [ ] PIE: fight a Ranger. Projectiles must still spawn from the correct socket.
- [ ] PIE: fight Baby. Its wave beam must still originate at its muzzle.

## 0.2 Regression check — Task 5 changed WaveAttack damage

Damage used to be nested inside `if (WaveEffect)`. It now always runs.

- [ ] PIE: let Baby hit you with its wave. You take damage and `LogR1` shows
      `WaveAttack (Laser): Hit 1 unique players`.
- [ ] Duplicate `GA_BabyWaveAttack` → **`GA_TestWaveNoVFX`**, clear its **Wave Effect**
      property, and temporarily swap it into `BP_Baby` → **Additional Skill Abilities**
      in place of the original.
- [ ] PIE: it must now deal damage with no beam particle. Before this fix it dealt zero.
- [ ] Restore `GA_BabyWaveAttack` in `BP_Baby`. Delete `GA_TestWaveNoVFX`.

## 0.3 Cooldowns — Tasks 1 and 2 together

- [ ] Create **`GE_BossSkillCooldown`** in `Content/Blueprints/AbilitySystem/GE/`:
  - Duration Policy: `Has Duration`
  - Duration Magnitude: `Set By Caller`, Data Tag `Data.Skill.Cooldown`
  - No modifiers, no granted tags (children add their own)
- [ ] `DT_BossSkillData` → row `GroundAttack` → set **Cooldown** to `10.0`
- [ ] **`BP_R1GameInstance` → `Boss Skill Data Table` = `DT_BossSkillData`** — do this
      before anything else, or the row below is never read and Cooldown stays 0 (mistake #0)
- [ ] `Cooldown.Boss.BabyGround` must already be declared in `R1GameplayTags`
      (done — see Appendix B; undeclared tags do not appear in the picker)
- [ ] New Blueprint Class with **parent `GE_BossSkillCooldown`** → **`GE_Cooldown_BabyGround`**,
      then **Components → Grant Tags to Target Actor → Add Tags → Added** =
      `Cooldown.Boss.BabyGround`
- [ ] `GA_BabyGroundAttack`:
  - **Cooldown Gameplay Effect Class** (category *Cooldowns*) = `GE_Cooldown_BabyGround`
  - **Activation Blocked Tags** (category *Tags*) += `Cooldown.Boss.BabyGround`
- [ ] PIE: Baby's ground attack must not *activate* twice within 10 seconds.
- [ ] PIE: during that window, `Selected Ability` lines name `GA_BabyWaveAttack_C`, and
      the `Candidates:` count drops from `2` to `1`.
- [ ] PIE: Baby never stands idle — it keeps attacking with the wave.
- [ ] Revert `DT_BossSkillData` row `GroundAttack` **Cooldown** to its shipping value
      (was `0`; leave `0` unless the designer wants otherwise). Keep the GE and the
      ability wiring — both are correct and stay.

## 0.4 Phase system, health gate, and transition montage — Task 3 (+ `fed8ebf`, `e1dd0e4`)

> **Verified working 2026-08-15** on Baby. Re-run only if the phase code changes.

- [ ] Open **`BB_Boss`** and add an **Int** key named exactly `Phase`.
      Nothing reads it yet; it exists for designer use in Behavior Trees.
- [ ] Create **`GE_TestEnrage`**: Infinite duration, one modifier —
      `UR1AttributeSet.MoveSpeed`, `Multiply`, `2.0`
- [ ] `BP_Baby` → **Phases** → add one entry: **Health Ratio Threshold** `0.5`,
      **Enrage Effect** `GE_TestEnrage`, **Transition Montage** = any Baby montage.
      *The montage is not optional for this test — with it empty the gate does nothing
      (mistake #4 above).*
- [ ] PIE: damage Baby past 50%. `LogR1` shows exactly one
      `entered boss phase 0 (threshold 0.50)`, and Baby visibly doubles speed.
- [ ] PIE: keep hitting it — that line must **not** repeat.

**Health pins at the threshold**
- [ ] PIE: hit Baby with an attack big enough to cross 50% from above. The HP bar must
      stop **exactly** on 50%, not below. Overkill damage past the gate is discarded.
- [ ] PIE: keep attacking during the transition. Damage numbers still appear, but the
      bar does not move until the montage finishes.

**Transition montage always plays**
- [ ] PIE: `LogR1` shows the pair
      `phase transition montage '<name>' started (<duration>s)` then
      `phase transition montage ended (interrupted: 0)`, roughly the montage duration apart.
- [ ] PIE: the montage runs start to finish — Baby does not attack, slide, or hit-react
      through it, even under continuous damage.
- [ ] PIE: the moment it ends, Baby resumes attacking and the HP bar can drop again.
      **A boss that stays passive means the activation inhibit never cleared — report it,
      it is a code bug, not a setup problem.**

> **Not testable: crossing two thresholds in one hit.** Earlier drafts of this checklist
> asked for that. The health floor now stops the hit at the first gate, so a single hit
> can never advance two phases. The `while` loop in `AdvancePhasesForRatio` is retained
> only as a safety net.

## 0.5 New abilities — Tasks 6 and 7

- [ ] Create **`GA_TestBossLeap`** (Blueprint child of `UR1GameplayAbility_BossLeap`):
  - **Skill ID** `GroundAttack` (reuses an existing row so damage is non-zero)
  - **Montage To Play** = any Baby attack montage
  - **Damage Effect** / **Telegraph Data** / **Telegraph Actor Class** = same as `GA_BabyGroundAttack`
  - **Dash Duration** `0.6`
- [ ] Add it to `BP_Baby` → **Additional Skill Abilities**. PIE from far away.
- [ ] PIE: Baby leaps at you and tracks your movement mid-air. `LogR1` shows
      `BossLeap: landed, hit 1 actors` when it connects, `hit 0 actors` when you dodge.
      A telegraph decal appears at your position as the leap starts.
- [ ] PIE: verify the no-target path — kill the player, or clear the blackboard, and
      force the ability. Expect `[BossLeap] blackboard key 'TargetActor' has no target
      actor`, a clean end, no crash.
- [ ] Create **`GA_TestSummon`** (child of `UR1GameplayAbility_SummonAdds`):
  - **Skill ID** `GroundAttack`, **Montage To Play** = any Baby montage
      *(the montage needs an `Event.Montage.Attack` notify — spawning happens there)*
  - **Add Classes** `[BP_Minion]`, **Spawn Count** `2`, **Alive Cap** `4`,
    **Spawn Ring Radius** `400`
- [ ] Add to `BP_Baby` → **Additional Skill Abilities**. PIE at range.
- [ ] PIE: `SummonAdds: spawned 2 adds (2 alive, cap 4)` then `(4 alive, cap 4)`.
      The ability then stops being selected until you kill an add.
- [ ] PIE: adds land on walkable ground, never inside walls, and chase normally.
- [ ] Equip the player's chain lightning and hit a monster — bounce behaviour unchanged,
      no `no initial target` warning. **This is the regression check for Task 7.**
- [ ] Duplicate `GA_ChainLightning` → **`GA_TestBossChain`**, add to `BP_Baby` →
      **Additional Skill Abilities**. PIE: lightning bounces starting from the player.

## 0.6 Clean up

- [ ] Remove `GA_TestBossLeap`, `GA_TestSummon`, `GA_TestBossChain` from `BP_Baby` and
      delete all three assets
- [ ] Clear the **Phases** array on `BP_Baby` (Baby ships unphased) and delete `GE_TestEnrage`
- [ ] Confirm `BP_Baby` still lists only its original abilities
- [ ] Commit the surviving assets: `GE_BossSkillCooldown`, `GE_Cooldown_BabyGround`,
      the `GA_BabyGroundAttack` wiring, the `BB_Boss` `Phase` key, and the `BP_Baby`
      muzzle socket fix

---

# Phase 1 — Shared setup

- [ ] **`BP_R1GameInstance` → Data → `Boss Skill Data Table` = `DT_BossSkillData`.**
      Nothing below works without this — see mistake #0. Find the class via
      Project Settings → Maps & Modes → Game Instance Class.
- [ ] Create folder `Content/Data/Telegraph/` (if absent)
- [ ] Create folder `Content/Blueprints/AbilitySystem/GE/Boss/`
- [ ] Create folders `Content/Blueprints/AbilitySystem/GA/Monster/Warden/`,
      `.../Ravager/`, `.../Hierarch/`
- [ ] Confirm `GE_BossSkillCooldown` exists from Phase 0.3 — every cooldown GE below is
      a **child** of it

**Cooldown GE recipe** (used ~17 times below): create a Blueprint Class whose **parent is
`GE_BossSkillCooldown`** — not a duplicate. Then set **Components → Grant Tags to Target
Actor → Add Tags → Added** to the one tag named in the table. Nothing else changes; the
duration is inherited and filled from the DataTable at runtime.

Child classes over duplicates because a later fix to the parent's Duration settings then
reaches all ~17 at once instead of needing seventeen edits.

⚠️ **The tag must already be declared in `R1GameplayTags`** or it will not appear in the
picker — see Appendix B. Only `Cooldown.Boss.BabyGround`, `.RavagerCharge` and
`.HierarchCharge` are declared so far.

---

# Phase 2 — 2F Zoner: Warden

Parent C++ class: `ABossWarden`. Identity: punishes both standing still and hugging.

## 2.1 DataTable rows — `DT_BossSkillData`

`ManaCost` is `0` for every boss skill (`BossAttackBase::CheckCost` is a no-op stub).
Damage values are opening guesses for tuning.

- [ ] Add these four rows:

| Row Name | Damage | ManaCost | Cooldown | Range |
|---|---|---|---|---|
| `WardenRepulse` | 30 | 0 | 6.0 | 350 |
| `WardenVolley` | 18 | 0 | 4.0 | 1800 |
| `WardenBeam` | 45 | 0 | 9.0 | 1400 |
| `WardenBeamWide` | 45 | 0 | 9.0 | 1400 |

## 2.2 Telegraph DataAssets — `Content/Data/Telegraph/`

⚠️ **These are hitbox values, not decoration.** `GroundAttack` and `BossLeap` read
`TelegraphSize.X` as the damage radius; `WaveAttack` reads `X` as length and `Y` as width.
Changing the visual size changes what the attack hits.

- [ ] Create three `UR1TelegraphData` assets:

| Asset | Shape | Telegraph Size | Duration | Decal Material |
|---|---|---|---|---|
| **`DA_Telegraph_Warden_Repulse`** | `Circle` | (350, 350) | 1.0 | `M_BossTelegraphCircle` |
| **`DA_Telegraph_Warden_Beam`** | `Rectangle` | (1400, 250) | 1.5 | `M_BossTelegraphRectangle` |
| **`DA_Telegraph_Warden_BeamWide`** | `Rectangle` | (1400, 500) | 1.0 | `M_BossTelegraphRectangle` |

## 2.3 Cooldown GEs — `Content/Blueprints/AbilitySystem/GE/Boss/`

- [ ] Create four, per the Phase 1 recipe:

| Asset | Granted tag |
|---|---|
| **`GE_Cooldown_WardenRepulse`** | `Cooldown.Boss.WardenRepulse` |
| **`GE_Cooldown_WardenVolley`** | `Cooldown.Boss.WardenVolley` |
| **`GE_Cooldown_WardenBeam`** | `Cooldown.Boss.WardenBeam` |
| **`GE_Cooldown_WardenBeamWide`** | `Cooldown.Boss.WardenBeamWide` |

## 2.4 Enrage GE

- [ ] Create **`GE_WardenEnrage`**: Infinite duration, one modifier —
      `UR1AttributeSet.AttackSpeed`, `Multiply`, `1.25`

## 2.5 Ability Blueprints — `Content/Blueprints/AbilitySystem/GA/Monster/Warden/`

Every one of these needs, in addition to the properties listed:
**Cooldown Gameplay Effect Class** = its GE from 2.3, and that GE's tag added to
**Activation Blocked Tags**.

- [ ] **`GA_WardenRepulse`** — parent `UR1GameplayAbility_GroundAttack`
  - **Skill ID** `WardenRepulse`
  - **Telegraph Data** `DA_Telegraph_Warden_Repulse`
  - **Telegraph Actor Class** = same class `GA_BabyGroundAttack` uses
  - **Damage Effect** = the boss damage GE
  - **Montage To Play** = 2F melee/slam montage — **needs `Event.Montage.Attack` notify**

- [ ] **`GA_WardenVolley`** — parent `UR1GameplayAbility_RangedAttack`
  - **Skill ID** `WardenVolley`
  - **Combo Sections** (category *Animation|Combo*) = `[Shot1, Shot2, Shot3]`
        — match the montage's actual section names
  - **Damage Effect**, **Fired Effect** (muzzle VFX)
  - **Montage To Play** — **needs one `Event.Montage.Attack` notify per section**
  - No telegraph

- [ ] **`GA_WardenBeam`** — parent `UR1GameplayAbility_WaveAttack`
  - **Skill ID** `WardenBeam`
  - **Telegraph Data** `DA_Telegraph_Warden_Beam`, **Telegraph Actor Class** as above
  - **Damage Effect**, **Wave Effect** (beam particle with a `BeamTarget` vector param)
  - **Montage To Play** — **needs notify**

- [ ] **`GA_WardenBeamWide`** — parent `UR1GameplayAbility_WaveAttack`
  - Identical to `GA_WardenBeam` except **Skill ID** `WardenBeamWide`,
    **Telegraph Data** `DA_Telegraph_Warden_BeamWide`, and its own cooldown GE + blocked tag

- [ ] **`GA_WardenHitReact`** — same parent class as `GA_BabyHitReact`; copy its settings
      and swap the montage. No cooldown, no blocked tag.

## 2.6 `BP_Warden` — `Content/Blueprints/Character/`

- [ ] Create a Blueprint child of **`ABossWarden`**

**Art and stats**
- [ ] Skeletal Mesh + Anim Class from the new 2F art
- [ ] **Character Row Name** = the boss's row in `CharacterStatTable`.
      *Add that row if it doesn't exist* — it supplies Health, MaxHealth, AttackRange,
      MoveSpeed, and AttackSpeed. A missing row means a boss with no stats.
- [ ] **Monster Init Stat Effect Class** = same as `BP_Baby`
- [ ] **Capsule** sized to the mesh (drives the AI's distance math in `BTService_PrepareSkill`)

**Combat**
- [ ] **Projectile Class** = the 2F projectile Blueprint
- [ ] **Muzzle Socket Name** = the socket name on the new mesh (default is `Muzzle_Front`)

**AI**
- [ ] **Default Behavior Tree** = `BT_Warden` (created in 2.7)

**Abilities**
- [ ] **Default Skill Abilities** = `[GA_WardenRepulse]`
- [ ] **Additional Skill Abilities** = `[GA_WardenVolley, GA_WardenBeam]`
- [ ] HitReact ability wired the same way `BP_Baby` wires `GA_BabyHitReact`

**Phases** — one entry
- [ ] **Health Ratio Threshold** `0.55`
- [ ] **Default Skills** — leave empty (in-range behaviour unchanged)
- [ ] **Additional Skills** = `[GA_WardenVolley, GA_WardenBeam, GA_WardenBeamWide]`
- [ ] **Enrage Effect** = `GE_WardenEnrage`
- [ ] **Transition Montage** = the 2F phase-change montage. **Required for the gate** —
      leave it empty and health will not pin at 55%, the boss keeps attacking through the
      swap, and the phase change reads as nothing happening (mistake #4).

**Rewards and feedback** (mirror `BP_Baby`)
- [ ] **Gold Actor Class**, **Min/Max Gold Drop**, **Gold Drop Chance**
- [ ] **Xp Effect**, **Death Anim Montage**, **Death Sound**
- [ ] **Hit Flash Overlay Material**, **Dissolve Materials**

## 2.7 `BT_Warden`

- [ ] Duplicate `BT_Baby` → **`BT_Warden`**
- [ ] Confirm it still uses `BB_Boss`
- [ ] Confirm the `BTService_PrepareSkill` node's **BBKey_TargetActor** and
      **BBKey_TargetAbilityClass** selectors still resolve to the right `BB_Boss` keys
      (duplication can leave them unset)

## 2.8 Room data

- [ ] Open `Content/Data/RoomDatas/Boss/DA_Boss_2F.uasset`
- [ ] Point its boss/monster reference at `BP_Warden`
- [ ] Point its room level at the **2F arena gameplay map** — must contain a
      `DungeonManager` and a `NavMeshBoundsVolume`, and the `DungeonManager` and doors
      must sit at the room sublevel's origin or `RegisterRoomManager`'s location match
      fails and the doors never activate

## 2.9 Verify Warden in PIE

- [ ] Boss HUD bar appears on entering the room
- [ ] At range, `GA_WardenVolley_C` and `GA_WardenBeam_C` alternate, neither repeating
      inside its cooldown
- [ ] Beam draws its rectangle decal ~1.5s before damage; outside it you take nothing,
      inside you take ~45
- [ ] Walking into melee triggers `GA_WardenRepulse_C` with a circular telegraph
- [ ] Below 55% health: exactly one `entered boss phase 0 (threshold 0.55)` line,
      attacks visibly speed up, `GA_WardenBeamWide_C` starts appearing
- [ ] The HP bar stops **exactly** on 55%, the transition montage plays start to finish
      (`interrupted: 0`), and Warden resumes attacking the moment it ends
- [ ] On death: HUD bar clears, gold drops, XP awarded
- [ ] No `ensureMsgf` about phase ordering at spawn

---

# Phase 3 — 3F Bruiser: Ravager

Parent C++ class: `ABossRavager`. Identity: constant pressure, punishes greed.

## 3.1 DataTable rows — `DT_BossSkillData`

- [ ] Add these six rows:

| Row Name | Damage | ManaCost | Cooldown | Range |
|---|---|---|---|---|
| `RavagerCombo` | 22 | 0 | 2.0 | 250 |
| `RavagerComboRage` | 26 | 0 | 2.0 | 250 |
| `RavagerLeap` | 40 | 0 | 8.0 | 1200 |
| `RavagerLeapRage` | 40 | 0 | 4.0 | 1200 |
| `RavagerCharge` | 38 | 0 | 7.0 | 1000 |
| `RavagerShockwave` | 50 | 0 | 10.0 | 600 |

## 3.2 Telegraph DataAssets

- [ ] Create three:

| Asset | Shape | Telegraph Size | Duration | Decal Material |
|---|---|---|---|---|
| **`DA_Telegraph_Ravager_Leap`** | `Circle` | (300, 300) | 0.8 | `M_BossTelegraphCircle` |
| **`DA_Telegraph_Ravager_Charge`** | `Rectangle` | (1000, 200) | 1.0 | `M_BossTelegraphRectangle` |
| **`DA_Telegraph_Ravager_Shockwave`** | `Circle` | (600, 600) | 1.2 | `M_BossTelegraphCircle` |

## 3.3 Cooldown GEs

- [ ] Create six, per the Phase 1 recipe — one per row in 3.1, each granting
      `Cooldown.Boss.<RowName>`:
      `GE_Cooldown_RavagerCombo`, `GE_Cooldown_RavagerComboRage`,
      `GE_Cooldown_RavagerLeap`, `GE_Cooldown_RavagerLeapRage`,
      `GE_Cooldown_RavagerCharge`, `GE_Cooldown_RavagerShockwave`

## 3.4 Enrage GEs

- [ ] **`GE_RavagerEnrage`**: Infinite; `UR1AttributeSet.MoveSpeed` `Multiply` `1.2`,
      and `UR1AttributeSet.AttackSpeed` `Multiply` `1.3`
- [ ] **`GE_RavagerFrenzy`**: Infinite; `UR1AttributeSet.AttackSpeed` `Multiply` `1.2`

> These **stack** — `EnterPhase` never removes an earlier phase's GE. Net attack speed
> in phase 3 is `1.3 × 1.2 = 1.56×`. Confirm that's where the designer wants it before
> tuning anything else.

## 3.5 Ability Blueprints — `.../GA/Monster/Ravager/`

Same rule as 2.5: each gets its cooldown GE **and** that GE's tag in
**Activation Blocked Tags**.

- [ ] **`GA_RavagerCombo`** — parent `UR1GameplayAbility_MonsterComboAttack`
  - **Skill ID** `RavagerCombo`
  - **Combo Sections** `[Combo1, Combo2, Combo3]` — match the montage's section names
  - **Damage Effect**, **Montage To Play** — **one notify per section**

- [ ] **`GA_RavagerComboRage`** — parent `UR1GameplayAbility_MonsterComboAttack`
  - **Skill ID** `RavagerComboRage`
  - **Combo Sections** `[Combo1, Combo2, Combo3, Combo4]`
  - Otherwise as above, own cooldown GE + blocked tag

- [ ] **`GA_RavagerLeap`** — parent `UR1GameplayAbility_BossLeap`
  - **Skill ID** `RavagerLeap`
  - **Telegraph Data** `DA_Telegraph_Ravager_Leap`, **Telegraph Actor Class** as Baby's
  - **Dash Duration** `0.6`, **Jump Height Curve** — optional arc curve
  - **BBKey Target Actor** — leave `TargetActor` unless `BB_Boss` renamed the key
  - **Damage Effect**, **Montage To Play**
  - ⚠️ **No AnimNotify needed** — `BossLeap` applies damage on root-motion finish,
    not on a montage event. Adding one does nothing.

- [ ] **`GA_RavagerLeapRage`** — identical to `GA_RavagerLeap` except **Skill ID**
      `RavagerLeapRage` and its own cooldown GE + blocked tag

- [ ] **`GA_RavagerCharge`** — parent `UR1GameplayAbility_WaveAttack`
  - **Skill ID** `RavagerCharge`, **Telegraph Data** `DA_Telegraph_Ravager_Charge`
  - **Damage Effect**, **Wave Effect**, **Montage To Play** — **needs notify**

- [ ] **`GA_RavagerShockwave`** — parent `UR1GameplayAbility_GroundAttack`
  - **Skill ID** `RavagerShockwave`, **Telegraph Data** `DA_Telegraph_Ravager_Shockwave`
  - **Damage Effect**, **Montage To Play** — **needs notify**

- [ ] **`GA_RavagerHitReact`** — copy `GA_BabyHitReact`, swap the montage

## 3.6 `BP_Ravager`

- [ ] Create a Blueprint child of **`ABossRavager`**
- [ ] Art, **Character Row Name**, **Monster Init Stat Effect Class**, capsule, rewards,
      death, and hit-flash settings — same checklist as 2.6
- [ ] **Default Behavior Tree** = `BT_Ravager`
- [ ] No **Projectile Class** needed (Ravager has no ranged attack)
- [ ] **Default Skill Abilities** = `[GA_RavagerCombo]`
- [ ] **Additional Skill Abilities** = `[GA_RavagerLeap, GA_RavagerCharge]`

**Phases — two entries, in descending threshold order.** Authoring them out of order
trips an `ensureMsgf` at `BeginPlay` and the later phase never fires.

- [ ] Entry 0: **Health Ratio Threshold** `0.50`
  - **Default Skills** `[GA_RavagerComboRage]`
  - **Additional Skills** `[GA_RavagerLeap, GA_RavagerCharge, GA_RavagerShockwave]`
  - **Enrage Effect** `GE_RavagerEnrage`
  - **Transition Montage** = the 3F phase-change montage — **required**, see mistake #4
- [ ] Entry 1: **Health Ratio Threshold** `0.20`
  - **Default Skills** — empty (keeps the 4-section combo)
  - **Additional Skills** `[GA_RavagerLeapRage, GA_RavagerCharge, GA_RavagerShockwave]`
  - **Enrage Effect** `GE_RavagerFrenzy`
  - **Transition Montage** = the 3F desperation montage — **required**, see mistake #4

## 3.7 `BT_Ravager`

- [ ] Duplicate `BT_Baby` → **`BT_Ravager`**, same key-selector verification as 2.7

## 3.8 Room data

- [ ] `DA_Boss_3F` → boss reference `BP_Ravager`, room level = 3F arena **gameplay** map
      (same `DungeonManager` / NavMeshBoundsVolume requirement as 2.8)

## 3.9 Verify Ravager in PIE

- [ ] In melee, `GA_RavagerCombo_C` chains three sections, each landing damage
- [ ] At range, `GA_RavagerLeap_C` closes the gap and visibly tracks you mid-leap
- [ ] `BossLeap: landed, hit 1 actors` when it connects, `hit 0 actors` when you dodge
- [ ] Below 50%: one `entered boss phase 0 (threshold 0.50)`, movement and attacks speed
      up, combo gains a 4th swing, shockwave starts appearing
- [ ] Below 20%: one `entered boss phase 1 (threshold 0.20)`, leaps come ~twice as often
- [ ] Both gates pin the HP bar exactly on 50% and 20%, and both transition montages play
      to completion (`interrupted: 0`) with Ravager resuming immediately after
- [ ] No phase-ordering `ensureMsgf` at spawn

---

# Phase 4 — 5F Summoner: Hierarch

Parent C++ class: `ABossHierarch`. Identity: final boss, teaches target priority.

## 4.1 DataTable rows — `DT_BossSkillData`

- [ ] Add these seven rows. `Damage = 0` on the summon rows is correct — `SummonAdds`
      never applies a damage effect.

| Row Name | Damage | ManaCost | Cooldown | Range |
|---|---|---|---|---|
| `HierarchStaff` | 35 | 0 | 3.0 | 300 |
| `HierarchSummon` | 0 | 0 | 20.0 | 2000 |
| `HierarchSummonMass` | 0 | 0 | 12.0 | 2000 |
| `HierarchBolt` | 25 | 0 | 5.0 | 1800 |
| `HierarchNova` | 60 | 0 | 15.0 | 900 |
| `HierarchNovaFast` | 60 | 0 | 9.0 | 900 |
| `HierarchChain` | 30 | 0 | 12.0 | 1500 |

## 4.2 Telegraph DataAssets

- [ ] Create two. Staff, Bolt, and Summon get no telegraph — leave **Telegraph Data** empty
      on those abilities.

| Asset | Shape | Telegraph Size | Duration | Decal Material |
|---|---|---|---|---|
| **`DA_Telegraph_Hierarch_Nova`** | `Circle` | (900, 900) | 2.5 | `M_BossTelegraphCircle` |
| **`DA_Telegraph_Hierarch_NovaFast`** | `Circle` | (900, 900) | 1.5 | `M_BossTelegraphCircle` |

## 4.3 Cooldown GEs

- [ ] Create six by the Phase 1 recipe, granting `Cooldown.Boss.<RowName>`:
      `GE_Cooldown_HierarchStaff`, `GE_Cooldown_HierarchSummon`,
      `GE_Cooldown_HierarchSummonMass`, `GE_Cooldown_HierarchBolt`,
      `GE_Cooldown_HierarchNova`, `GE_Cooldown_HierarchNovaFast`

- [ ] Create **`GE_Cooldown_HierarchChain`** **differently** — this one does **not** use
      `Set By Caller`:
  - Duration Policy `Has Duration`, Duration Magnitude **`Scalable Float` = `12.0`**
  - Grant Tags to Target Actor: `Cooldown.Boss.HierarchChain`

> ⚠️ **Why the exception:** `GA_HierarchChain`'s parent
> `UR1GameplayAbility_ChainLightning` extends `UR1GameplayAbility`, **not**
> `BossAttackBase`, so it does not inherit the `ApplyCooldown` override and will never
> inject the DataTable value. A `Set By Caller` duration with no caller resolves to 0 and
> the cooldown silently does nothing. The `HierarchChain` row's `Cooldown` column stays as
> documentation only — the real number lives in this GE.

## 4.4 Enrage GE

- [ ] **`GE_HierarchEnrage`**: Infinite; `UR1AttributeSet.AttackSpeed` `Multiply` `1.15`

## 4.5 Ability Blueprints — `.../GA/Monster/Hierarch/`

Same rule as 2.5 for cooldown GE + blocked tag on every ability.

- [ ] **`GA_HierarchStaff`** — parent `UR1GameplayAbility_MonsterMeeleAttack`
  - **Skill ID** `HierarchStaff`, **Damage Effect**, **Montage To Play** — **needs notify**

- [ ] **`GA_HierarchSummon`** — parent `UR1GameplayAbility_SummonAdds`
  - **Skill ID** `HierarchSummon`
  - **Add Classes** `[BP_Minion, BP_TaintedMinion]`
  - **Spawn Count** `2`, **Alive Cap** `4`, **Spawn Ring Radius** `400`
  - **Montage To Play** — ⚠️ **needs an `Event.Montage.Attack` notify.** Spawning happens
    in `OnAttackEventReceived`; with no notify, nothing spawns and nothing logs.

- [ ] **`GA_HierarchSummonMass`** — same parent
  - **Skill ID** `HierarchSummonMass`, **Spawn Count** `3`, **Alive Cap** `6`
  - Same montage + notify requirement, own cooldown GE + blocked tag

- [ ] **`GA_HierarchBolt`** — parent `UR1GameplayAbility_RangedAttack`
  - **Skill ID** `HierarchBolt`, **Damage Effect**, **Fired Effect**
  - **Montage To Play** — **needs notify**

- [ ] **`GA_HierarchNova`** — parent `UR1GameplayAbility_GroundAttack`
  - **Skill ID** `HierarchNova`, **Telegraph Data** `DA_Telegraph_Hierarch_Nova`
  - **Damage Effect**, **Montage To Play** — **needs notify**

- [ ] **`GA_HierarchNovaFast`** — same parent
  - **Skill ID** `HierarchNovaFast`, **Telegraph Data** `DA_Telegraph_Hierarch_NovaFast`
  - Own cooldown GE + blocked tag

- [ ] **`GA_HierarchChain`** — parent `UR1GameplayAbility_ChainLightning`
  - **Skill ID** `HierarchChain`, **Max Bounces** `4`, **Bounce Radius** `800`
  - **Lightning Effect** (damage GE), **Lightning VFX** (Niagara)
  - **Cooldown Gameplay Effect Class** = `GE_Cooldown_HierarchChain` (the fixed-duration one)
  - **Activation Blocked Tags** += `Cooldown.Boss.HierarchChain`

- [ ] **`GA_HierarchHitReact`** — copy `GA_BabyHitReact`, swap the montage

## 4.6 `BP_Hierarch`

- [ ] Create a Blueprint child of **`ABossHierarch`**
- [ ] Art, **Character Row Name**, **Monster Init Stat Effect Class**, capsule, rewards,
      death, hit-flash — same checklist as 2.6
- [ ] **Default Behavior Tree** = `BT_Hierarch`
- [ ] **Projectile Class** = the 5F bolt projectile; **Muzzle Socket Name** = its socket
- [ ] **Default Skill Abilities** = `[GA_HierarchStaff]`
- [ ] **Additional Skill Abilities** =
      `[GA_HierarchSummon, GA_HierarchBolt, GA_HierarchNova]`

**Phases — two entries, descending order**
- [ ] Entry 0: **Health Ratio Threshold** `0.66`
  - **Default Skills** — empty
  - **Additional Skills**
    `[GA_HierarchSummon, GA_HierarchBolt, GA_HierarchNova, GA_HierarchChain]`
  - **Enrage Effect** `GE_HierarchEnrage`
  - **Transition Montage** = the 5F phase-change montage — **required**, see mistake #4
- [ ] Entry 1: **Health Ratio Threshold** `0.33`
  - **Default Skills** — empty
  - **Additional Skills**
    `[GA_HierarchSummonMass, GA_HierarchBolt, GA_HierarchNovaFast, GA_HierarchChain]`
  - **Enrage Effect** — none
  - **Transition Montage** = the 5F final-stand montage — **required**, see mistake #4

## 4.7 `BT_Hierarch`

- [ ] Duplicate `BT_Baby` → **`BT_Hierarch`**, same key-selector verification as 2.7

## 4.8 Room data

- [ ] `DA_Boss_5F` → boss reference `BP_Hierarch`, room level = 5F arena **gameplay** map

## 4.9 Verify Hierarch in PIE

- [ ] At range, summon fires early: `SummonAdds: spawned 2 adds (2 alive, cap 4)`;
      adds land on walkable ground and chase
- [ ] With 4 adds alive, summon stops being selected until you kill some —
      the `Candidates:` count drops by one
- [ ] Nova draws its large circle 2.5s ahead; leaving the circle avoids all 60 damage
- [ ] Below 66%: one `entered boss phase 0 (threshold 0.66)`, chain lightning appears
      and bounces from you to nearby adds
- [ ] Below 33%: one `entered boss phase 1 (threshold 0.33)`, summons come in 3s up to
      6 alive, nova telegraph shortens to 1.5s
- [ ] Both gates pin the HP bar exactly on 66% and 33%, and both transition montages play
      to completion (`interrupted: 0`). Note adds keep attacking during the transition —
      only the boss is inhibited, which is intended.
- [ ] Chain lightning respects its 12s cooldown (this is the check that 4.3's
      fixed-duration GE is wired correctly — if it spams, that GE is wrong)

### ⚠️ 4.10 The one genuinely uncertain interaction

`SummonAdds` calls `InitializeWithManager(Boss->OwningDungeonManager)` on every add, so
the room's `DungeonManager` counts them as room monsters. The room-clear condition may
therefore wait for adds that outlive the boss.

- [ ] Kill the Hierarch while adds are still alive. Does the door open?
- [ ] **If yes** — nothing to do.
- [ ] **If no** — this is a real bug, not a tuning issue. Decide with the designer:
      either adds should count toward room clear (then the boss must despawn them on
      death), or they shouldn't be registered with the manager at all. Either fix is a
      C++ change; report it rather than working around it in Blueprints.

---

# Phase 5 — Final integration pass

- [ ] **Baby (1F) unchanged** — fight it start to finish. Its `Phases` array is empty and
      it should behave exactly as before this branch.
- [ ] **Ranger monsters** still fire projectiles from the correct socket (Task 4)
- [ ] **Player chain lightning** still triggers on weapon hit (Task 7)
- [ ] **No boss stands idle.** If every skill is on cooldown, `BTService_PrepareSkill`
      leaves the blackboard untouched and `BTTask_ExecuteSkill` returns `Failed`; the
      tree should retry next tick. If a boss freezes mid-fight instead, its BT needs a
      `Wait` fallback node beside the execute task.
- [ ] **Full run** — play 1F → 5F and confirm each boss room loads, registers, and clears
- [ ] Commit all new assets

---

# Appendix A — Asset inventory

| Kind | Count | Notes |
|---|---|---|
| `DT_BossSkillData` rows | 17 | 4 Warden, 6 Ravager, 7 Hierarch |
| Telegraph DataAssets | 8 | 3 Warden, 3 Ravager, 2 Hierarch |
| Cooldown GEs | 19 | `GE_BossSkillCooldown` base + `BabyGround` + 4 Warden + 6 Ravager + 7 Hierarch; `HierarchChain` alone is fixed-duration |
| Enrage GEs | 4 | Warden 1, Ravager 2, Hierarch 1 |
| Ability Blueprints | 20 | incl. 3 HitReact |
| Boss Blueprints | 3 | `BP_Warden`, `BP_Ravager`, `BP_Hierarch` |
| Behavior Trees | 3 | duplicates of `BT_Baby` |
| Blackboard keys | 1 | `Phase` (Int) on `BB_Boss` |
| `CharacterStatTable` rows | 3 | one per boss — easy to forget, boss has no stats without it |
| RoomData assets edited | 3 | `DA_Boss_2F/3F/5F` |

# Appendix B — Gameplay tags introduced

⚠️ **Each of these must be declared natively in `R1GameplayTags` before it can be used.**
This project registers **no** tags through `DefaultGameplayTags.ini` (it is empty) — every
tag is a `UE_DECLARE_GAMEPLAY_TAG_EXTERN` / `UE_DEFINE_GAMEPLAY_TAG` pair, as
`Cooldown.Skill.BladeWave` already is. An undeclared tag simply will not appear in the
editor's tag picker, so the GE cannot grant it and the ability cannot block on it.

Declare them in batches as each boss is built, not all at once — the boss names are still
placeholders, and renaming seventeen tags later is pure rework. Only
`Cooldown.Boss.BabyGround` is declared so far (for the Phase 0.3 smoke test).

Once declared, each is set on its GE via **Grant Tags to Target Actor** and on its ability
via **Activation Blocked Tags**.

```
Cooldown.Boss.BabyGround
Cooldown.Boss.WardenRepulse
Cooldown.Boss.WardenVolley
Cooldown.Boss.WardenBeam
Cooldown.Boss.WardenBeamWide
Cooldown.Boss.RavagerCombo
Cooldown.Boss.RavagerComboRage
Cooldown.Boss.RavagerLeap
Cooldown.Boss.RavagerLeapRage
Cooldown.Boss.RavagerCharge
Cooldown.Boss.RavagerShockwave
Cooldown.Boss.HierarchStaff
Cooldown.Boss.HierarchSummon
Cooldown.Boss.HierarchSummonMass
Cooldown.Boss.HierarchBolt
Cooldown.Boss.HierarchNova
Cooldown.Boss.HierarchNovaFast
Cooldown.Boss.HierarchChain
```

`Data.Skill.Cooldown` and `Event.Montage.Attack` already exist in `R1GameplayTags` — do
not redeclare them.

# Appendix C — Log lines to watch

| Line | Means |
|---|---|
| `BTService_PrepareSkill: Selected Ability X (CanAttack: N, Candidates: M)` | skill picked; `Candidates` shrinking proves cooldown filtering works |
| `BTService_PrepareSkill: no ability off cooldown` | Verbose level — everything is cooling down; boss will retry |
| `<Boss> entered boss phase N (threshold X)` | phase transition; must appear exactly once per threshold |
| `<Boss>: phase transition montage 'X' started (Ns)` | gate engaged — health pinned, all ability activation inhibited |
| `<Boss>: phase transition montage ended (interrupted: 0)` | gate released cleanly; `interrupted: 1` means something still overrode the montage |
| `<Boss>: phase transition montage failed to play` | `Montage_Play` returned 0 — bad montage asset or no anim instance; the gate is skipped |
| `BossSkillDataTable이 비어있어 SkillDataTable로 폴백합니다` | mistake #0 — `BP_R1GameInstance` still needs `DT_BossSkillData` |
| `[GA_X] SkillID 'Y' resolved: Damage=.. Cooldown=.. Range=..` | the row was found; these are the values actually in use |
| `[GA_X] SkillID 'Y' not found in the ... BossSkillDataTable` | row name ≠ the ability's `Skill ID`, or the table pointer is wrong |
| `[GA_X] CachedCooldown is 0 for SkillID 'Y'` | row Cooldown is 0 **or** the row was never found — check for a `resolved` line above |
| `[GA_X] no Cooldown Gameplay Effect Class assigned` | the ability has no cooldown GE, so it can never go on cooldown |
| `[GA_X] montage ended without ever receiving '...'` | mistake #1 — no notify, or its tag does not match (watch the `Monage` typo) |
| `BossCharge: hit <actor>` / `finished (targets hit: N)` | dash connected / completed. `Executing Ability` alone does **not** mean it activated — that line is printed before `TryActivateAbility`, so repeats are usually cooldown-blocked retries |
| `BossLeap: landed, hit N actors` | leap resolved |
| `[BossLeap] blackboard key 'TargetActor' has no target actor` | leap had no target, ended cleanly |
| `SummonAdds: spawned N adds (M alive, cap C)` | summon resolved |
| `[SummonAdds] nav projection failed at X, skipping` | a ring position was off-navmesh; reduce Spawn Ring Radius if frequent |
| `[ChainLightning] no initial target from event or blackboard` | chain lightning fired with nothing to hit |
| `WaveAttack (Laser): Hit N unique players` | rectangle attack resolved |
| `GroundAttack: Hit N actors` | circle attack resolved |
