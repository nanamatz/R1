# Monster Attack Hit Impact VFX Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** All monster attacks show a hit-impact Niagara at each damaged player, with the VFX chosen per attack GA.

**Architecture:** `UR1GameplayAbility` gains an optional `HitImpactVFX` (per-ability, set in the GA BP). Monster attack paths execute the existing `GameplayCue.Weapon.Impact` cue per damaged player, passing the ability VFX as `CueParams.SourceObject` (free in monster paths — their sound plays directly). The cue notify's resolve order becomes: SourceObject-as-Niagara → player weapon DA → `DefaultHitVFX`. Player paths are unchanged (their SourceObject is a sound; the Niagara cast fails harmlessly).

**Tech Stack:** UE 5.3, C++, GAS, Niagara. Build command and constraints: see `docs/superpowers/plans/2026-07-10-per-weapon-hit-vfx.md` Global Constraints (same rules: no test harness — compile gate + manual PIE; never `git add -A`; editor must be closed for header builds).

**Design approval:** conversation 2026-07-11 — per-GA management, all monster attacks, WaveAttack's legacy Cascade `ImpactEffect` replaced by the cue.

---

### Task 1: Base GA property + cue notify resolve order

**Files:**
- Modify: `Source/R1/AbilitySystem/Abilities/R1GameplayAbility.h` (~line 51, below `SoundToPlay`)
- Modify: `Source/R1/AbilitySystem/R1GameplayCueNotify_WeaponImpact.cpp` (VFX resolve block)

**Interfaces:**
- Produces: `protected TObjectPtr<UNiagaraSystem> HitImpactVFX` on `UR1GameplayAbility` (all attack GAs inherit); cue notify accepts a `UNiagaraSystem` in `Parameters.SourceObject` as highest-priority VFX.

- [ ] Add to `UR1GameplayAbility` below `SoundToPlay`:

```cpp
	// 명중 시 피격 지점에 재생할 어빌리티 전용 임팩트 이펙트.
	// 설정 시 GameplayCue.Weapon.Impact 큐에 SourceObject로 전달되어 무기 DA/기본 VFX보다 우선한다.
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UNiagaraSystem> HitImpactVFX;
```

- [ ] In `R1GameplayCueNotify_WeaponImpact.cpp`, change the VFX resolve to try SourceObject first:

```cpp
	// [VFX] 우선순위: 어빌리티 지정(SourceObject의 Niagara) → 장착 무기 DA → DefaultHitVFX
	UNiagaraSystem* ImpactVFX = Cast<UNiagaraSystem>(const_cast<UObject*>(Parameters.GetSourceObject()));
	if (!ImpactVFX)
	{
		if (AR1Player* Player = Cast<AR1Player>(Parameters.GetInstigator()))
		{
			if (UR1EquipmentManagerComponent* EquipManager = Player->GetEquipmentComponent())
			{
				ImpactVFX = EquipManager->GetHitImpactVFX(ER1EquipmentSlot::Weapon);
			}
		}
	}
	if (!ImpactVFX)
	{
		ImpactVFX = DefaultHitVFX;
	}
```

(Sound block unchanged — `Cast<USoundBase>` on a Niagara SourceObject returns null, no sound plays, monster sound keeps playing from the ability.)

### Task 2: Sector damage out-param + minion melee/combo wiring

**Files:**
- Modify: `Source/R1/Library/R1AbilitySystemLibrary.h:24` and `.cpp:117` — append `TArray<AActor*>* OutDamagedPlayers = nullptr`; in the damage loop, `if (OutDamagedPlayers) OutDamagedPlayers->Add(TargetPlayer);`
- Modify: `Source/R1/AbilitySystem/Abilities/R1GameplayAbility_MonsterMeeleAttack.cpp` — collect damaged players, fire cue per player (`SourceObject = HitImpactVFX`, `Instigator = SourceCharacter`, `Location =` player chest, `Normal =` player→monster). Combo attack inherits this.
- Includes: `R1GameplayTags.h`, `NiagaraSystem.h` in the melee cpp.

### Task 3: GroundAttack + WaveAttack wiring

**Files:**
- Modify: `Source/R1/AbilitySystem/Abilities/R1GameplayAbility_GroundAttack.cpp` — in the damage loop, fire the same cue per damaged player.
- Modify: `Source/R1/AbilitySystem/Abilities/R1GameplayAbility_WaveAttack.cpp` — replace the legacy Cascade `ImpactEffect` spawn block with the cue execution; `Source/R1/AbilitySystem/Abilities/R1GameplayAbility_WaveAttack.h` — delete the now-unused `ImpactEffect` property.

### Task 4: Build, commit, editor setup (user)

- [ ] Editor closed → full build (header changed) → commit code.
- [ ] User: set `HitImpactVFX` on GA BPs (monster melee/combo/ground/wave attacks; optionally player skills), re-save WaveAttack GA BP (property removal), PIE-verify: minion hit, combo, GroundAttack AoE, WaveAttack beam — each shows its GA's VFX (or `DefaultHitVFX` when unset) on the player; player-side behavior unchanged.
