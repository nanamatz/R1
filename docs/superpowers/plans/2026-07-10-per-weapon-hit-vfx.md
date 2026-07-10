# Per-Weapon Hit Impact VFX Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Melee hits play a one-shot Niagara impact VFX at the hit point, chosen per-weapon from the weapon's item Data Asset, with a project default fallback for unarmed/monster hits.

**Architecture:** Add a `HitImpactVFX` soft pointer to `UR1ItemAssetData` and a lookup on `UR1EquipmentManagerComponent` (mirroring the existing `GetSoundByTag` audio pattern). A new C++ GameplayCue notify (`UGameplayCueNotify_Static` subclass) resolves the VFX from the cue Instigator's equipped weapon at presentation time and spawns it at `Parameters.Location`. `GA_Attack` stops gating the `GameplayCue.Weapon.Impact` cue on the weapon sound existing, so the cue fires on every melee hit.

**Tech Stack:** UE 5.3, C++, GAS (GameplayCueNotify_Static), Niagara.

**Spec:** `docs/superpowers/specs/2026-07-10-per-weapon-hit-vfx-design.md`

## Global Constraints

- This UE project has **no automated test harness**. The verification gate per task is: the editor target compiles. Behavior verification is manual PIE in Task 4 (user-driven, requires the Unreal editor).
- Build command (verified for this machine):
  ```powershell
  & "C:\Program Files\Epic Games\UE_5.3\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "C:\Users\owner\Documents\GitHub\R1\R1.uproject" -waitmutex
  ```
  Success = output ends with `Total execution time`, exit code 0. Incremental builds take ~20-30s.
- Tasks 1 and 2 change headers → **Live Coding cannot apply them.** Before building, run `Get-Process UnrealEditor -ErrorAction SilentlyContinue`; if the editor is running, ask the user to close it (never kill the process).
- The working tree has an unrelated modified file (`Content/UI/System/GameOver/WBP_GameOver.uasset`). **Never `git add -A`** — stage only the files listed in each task.
- UObject members: raw pointer/`TObjectPtr` + `UPROPERTY()` only (project rule). No `HasAuthority()` branches, no RPCs (single-player project).
- Follow Epic naming: `U` prefix for UObject classes, `b` prefix for bools, PascalCase functions.
- Comments in this codebase are Korean — new comments follow suit.

---

### Task 1: `HitImpactVFX` data field + equipment lookup

**Files:**
- Modify: `Source/R1/Data/R1ItemAssetData.h` (after the `WeaponAuraVFX` property, ~line 83)
- Modify: `Source/R1/System/R1EquipmentManagerComponent.h` (after the `GetSoundByTag` declaration, ~line 46)
- Modify: `Source/R1/System/R1EquipmentManagerComponent.cpp` (after the `GetSoundByTag` definition, ~line 303)

**Interfaces:**
- Consumes: existing `EquippedItemsMap` (`TMap<ER1EquipmentSlot, TObjectPtr<UR1ItemAssetData>>`, private member of `UR1EquipmentManagerComponent`).
- Produces: `UNiagaraSystem* UR1EquipmentManagerComponent::GetHitImpactVFX(ER1EquipmentSlot EquipSlot) const` — returns the equipped item's loaded `HitImpactVFX`, or `nullptr` if the slot is empty or the field unset. Task 2 calls this.

- [ ] **Step 1: Add the field to `UR1ItemAssetData`**

In `Source/R1/Data/R1ItemAssetData.h`, directly below the `WeaponAuraVFX` property block:

```cpp
	// 명중 시 피격 지점에 재생되는 원샷 임팩트 이펙트 (미지정 시 GCN의 DefaultHitVFX 폴백)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
	TSoftObjectPtr<class UNiagaraSystem> HitImpactVFX;
```

- [ ] **Step 2: Declare the accessor**

In `Source/R1/System/R1EquipmentManagerComponent.h`, directly below the `GetSoundByTag` declaration:

```cpp
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	class UNiagaraSystem* GetHitImpactVFX(ER1EquipmentSlot EquipSlot) const;
```

- [ ] **Step 3: Define the accessor**

In `Source/R1/System/R1EquipmentManagerComponent.cpp`, directly below the `GetSoundByTag` definition (the file already includes `NiagaraSystem.h` and `Data/R1ItemAssetData.h`):

```cpp
UNiagaraSystem* UR1EquipmentManagerComponent::GetHitImpactVFX(ER1EquipmentSlot EquipSlot) const
{
	if (const TObjectPtr<UR1ItemAssetData>* FoundItem = EquippedItemsMap.Find(EquipSlot))
	{
		if (*FoundItem && !(*FoundItem)->HitImpactVFX.IsNull())
		{
			return (*FoundItem)->HitImpactVFX.LoadSynchronous();
		}
	}
	return nullptr;
}
```

- [ ] **Step 4: Verify editor is closed, then build**

Run: `Get-Process UnrealEditor -ErrorAction SilentlyContinue` — if it returns a process, ask the user to close the editor and wait.

Then run the build command from Global Constraints.
Expected: exit 0, output ends with `Total execution time`.

- [ ] **Step 5: Commit**

```powershell
git add Source/R1/Data/R1ItemAssetData.h Source/R1/System/R1EquipmentManagerComponent.h Source/R1/System/R1EquipmentManagerComponent.cpp
git commit -m "feat: add per-weapon HitImpactVFX field and equipment lookup"
```

---

### Task 2: Weapon-impact GameplayCue notify class

**Files:**
- Create: `Source/R1/AbilitySystem/R1GameplayCueNotify_WeaponImpact.h`
- Create: `Source/R1/AbilitySystem/R1GameplayCueNotify_WeaponImpact.cpp`

**Interfaces:**
- Consumes: `UR1EquipmentManagerComponent::GetHitImpactVFX(ER1EquipmentSlot) const` from Task 1; existing `AR1Player::GetEquipmentComponent()`.
- Produces: `UR1GameplayCueNotify_WeaponImpact` (extends `UGameplayCueNotify_Static`) with `UPROPERTY(EditDefaultsOnly) TObjectPtr<UNiagaraSystem> DefaultHitVFX`. Task 4 reparents the existing `GCN_WeaponImpact` BP to it.

Engine signature note (verified in `GameplayCueNotify_Static.h` of UE 5.3): `OnExecute` is a **const** BlueprintNativeEvent — the override must be `const` or it will not compile as an override.

- [ ] **Step 1: Write the header**

`Source/R1/AbilitySystem/R1GameplayCueNotify_WeaponImpact.h`:

```cpp

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "R1GameplayCueNotify_WeaponImpact.generated.h"

class UNiagaraSystem;

/**
 * 무기 명중 임팩트 큐(GameplayCue.Weapon.Impact): SourceObject로 전달된 무기 사운드를 재생하고,
 * 공격자(Instigator)의 장착 무기 DA에서 HitImpactVFX를 찾아 피격 지점(Parameters.Location)에 스폰한다.
 * 무기 VFX가 없으면(맨손/몬스터/구형 DA) DefaultHitVFX 폴백. BP(GCN_WeaponImpact)는 이 클래스로
 * 리페어런팅 후 GameplayCue Tag와 DefaultHitVFX만 설정하면 된다.
 */
UCLASS()
class R1_API UR1GameplayCueNotify_WeaponImpact : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

protected:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

	// 무기 DA에 HitImpactVFX가 없을 때 사용할 기본 임팩트 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "R1|VFX")
	TObjectPtr<UNiagaraSystem> DefaultHitVFX;
};
```

- [ ] **Step 2: Write the implementation**

`Source/R1/AbilitySystem/R1GameplayCueNotify_WeaponImpact.cpp`:

```cpp

#include "AbilitySystem/R1GameplayCueNotify_WeaponImpact.h"
#include "Character/R1Player.h"
#include "System/R1EquipmentManagerComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

bool UR1GameplayCueNotify_WeaponImpact::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	// 정적 큐는 CDO에서 실행되므로 월드는 대상 액터에서 얻는다
	if (!MyTarget || !MyTarget->GetWorld())
	{
		return false;
	}

	// [사운드] 기존 GCN_WeaponImpact BP 로직 이관: SourceObject의 무기 사운드 재생 (없으면 무음)
	if (USoundBase* Sound = Cast<USoundBase>(const_cast<UObject*>(Parameters.GetSourceObject())))
	{
		UGameplayStatics::PlaySoundAtLocation(MyTarget, Sound, Parameters.Location);
	}

	// [VFX] 공격자의 장착 무기 DA → HitImpactVFX, 없으면 DefaultHitVFX 폴백
	UNiagaraSystem* ImpactVFX = nullptr;
	if (AR1Player* Player = Cast<AR1Player>(Parameters.GetInstigator()))
	{
		if (UR1EquipmentManagerComponent* EquipManager = Player->GetEquipmentComponent())
		{
			ImpactVFX = EquipManager->GetHitImpactVFX(ER1EquipmentSlot::Weapon);
		}
	}
	if (!ImpactVFX)
	{
		ImpactVFX = DefaultHitVFX;
	}

	if (ImpactVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			MyTarget, ImpactVFX, Parameters.Location, Parameters.Normal.Rotation());
	}

	return false;
}
```

- [ ] **Step 3: Verify editor is closed, then build**

Run: `Get-Process UnrealEditor -ErrorAction SilentlyContinue` — if running, ask the user to close it.

Then run the build command from Global Constraints.
Expected: exit 0, `Total execution time`. If the compiler reports the override does not match (`const` mismatch), check the exact signature in `C:\Program Files\Epic Games\UE_5.3\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Public\GameplayCueNotify_Static.h` and match it verbatim.

- [ ] **Step 4: Commit**

```powershell
git add Source/R1/AbilitySystem/R1GameplayCueNotify_WeaponImpact.h Source/R1/AbilitySystem/R1GameplayCueNotify_WeaponImpact.cpp
git commit -m "feat: add weapon impact GameplayCue notify with per-weapon VFX"
```

---

### Task 3: Fire the impact cue on every melee hit

**Files:**
- Modify: `Source/R1/AbilitySystem/Abilities/R1GameplayAbility_Attack.cpp:165-214` (inside `OnAttackEventReceived`)

**Interfaces:**
- Consumes: nothing new — reuses the existing `WeaponSound` lookup, `GameplayCueTag`, and inherited `SoundToPlay` (declared on `UR1GameplayAbility`).
- Produces: behavior only — `GameplayCue.Weapon.Impact` now executes whenever `GameplayCueTag` is valid (with `SourceObject` possibly null), instead of only when a weapon sound exists.

Audio-preservation contract (all three old cases must behave identically for sound):
| Case | Old | New |
|---|---|---|
| WeaponSound + valid CueTag | cue plays WeaponSound | cue plays WeaponSound |
| no WeaponSound (unarmed/monster) | direct `SoundToPlay` | cue fires (silent) + direct `SoundToPlay` |
| WeaponSound but invalid CueTag | direct `SoundToPlay` | direct `SoundToPlay`, no cue |

- [ ] **Step 1: Restructure the cue/sound block**

In `R1GameplayAbility_Attack.cpp`, `OnAttackEventReceived`, replace the block starting at the `// [오디오] 1) 장착 무기 사운드 + GameplayCue...` comment (line 165) through the `else if (SoundToPlay) { ... }` closing brace (line 214) with:

```cpp
					// [오디오] 장착 무기 사운드 조회 (AudioRoutingMap 라우팅)
					USoundBase* WeaponSound = nullptr;
					if (AudioTag.IsValid())
					{
						if (AR1Player* Player = Cast<AR1Player>(SourceCharacter))
						{
							if (UR1EquipmentManagerComponent* EquipManager = Player->GetEquipmentComponent())
							{
								WeaponSound = EquipManager->GetSoundByTag(ER1EquipmentSlot::Weapon, AudioTag);
							}
						}
					}

					// [큐] 명중 시 항상 실행: 사운드(SourceObject, 없으면 무음) + 임팩트 VFX(GCN이 무기 DA에서 조회)
					if (GameplayCueTag.IsValid())
					{
						FGameplayCueParameters CueParams;
						CueParams.SourceObject = WeaponSound;
						CueParams.Instigator = SourceCharacter;

						FVector StartLoc = SourceCharacter->GetActorLocation() + FVector(0, 0, 50.0f); // 명치를 향하도록 Z축 보정
						FVector EndLoc = TargetActor->GetActorLocation() + FVector(0, 0, 50.0f);

						FHitResult HitResult;
						FCollisionQueryParams TraceParams;
						TraceParams.AddIgnoredActor(SourceCharacter);

						// 공격자의 명치에서 타겟의 명치로 보이지 않는 선을 긋습니다.
						bool bHit = SourceCharacter->GetWorld()->LineTraceSingleByChannel(
							HitResult, StartLoc, EndLoc, ECC_Visibility, TraceParams);

						if (bHit)
						{
							// 캡슐(피부)에 맞았다면 그 정확한 표면 지점과 각도를 사용합니다.
							CueParams.Location = HitResult.ImpactPoint;
							CueParams.Normal = HitResult.ImpactNormal;
						}
						else
						{
							// 만약 장애물 등으로 빗나갔다면(예외 상황) 기본 위치로 세팅
							CueParams.Location = TargetActor->GetActorLocation() + FVector(0, 0, 50.0f);
							CueParams.Normal = (StartLoc - EndLoc).GetSafeNormal();
						}

						SourceASC->ExecuteGameplayCue(GameplayCueTag, CueParams);
					}

					// [오디오 폴백] 큐가 무기 사운드를 재생하지 못하는 경우(맨손/몬스터/큐 태그 미설정) 기존 동작 유지
					if (!(WeaponSound && GameplayCueTag.IsValid()) && SoundToPlay)
					{
						UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, TargetActor->GetActorLocation());
					}
```

- [ ] **Step 2: Build**

.cpp-only change (Live Coding-compatible, but build anyway to gate the commit). Run the build command from Global Constraints.
Expected: exit 0, `Total execution time`.

- [ ] **Step 3: Commit**

```powershell
git add Source/R1/AbilitySystem/Abilities/R1GameplayAbility_Attack.cpp
git commit -m "feat: fire weapon impact cue on every melee hit"
```

---

### Task 4: Editor wiring + manual PIE verification (user-driven)

**Files:** editor-only (.uasset changes made by the user in the Unreal editor — not editable from CLI).

**Interfaces:**
- Consumes: `UR1GameplayCueNotify_WeaponImpact` (Task 2), `HitImpactVFX` field (Task 1).
- Produces: working in-game feature.

- [ ] **Step 1: Ask the user to perform the editor wiring**

Relay these steps (user opens the editor after the Task 3 build):

1. Open `Content/Blueprints/Audio/GCN/GCN_WeaponImpact` → **File > Reparent Blueprint** → `R1GameplayCueNotify_WeaponImpact`.
2. Delete the BP's old sound-playing graph logic (now duplicated by C++ — leaving it would double-play the sound). Confirm the **GameplayCue Tag** is still `GameplayCue.Weapon.Impact`.
3. In the BP's Class Defaults, set **DefaultHitVFX** to the chosen default impact Niagara system.
4. On each weapon DA that should have a unique impact, set **Equipment|Visual > HitImpactVFX**.
5. Save all.

- [ ] **Step 2: Ask the user to run the PIE checklist**

1. Hit a monster with a weapon that has `HitImpactVFX` set → that VFX appears at the impact point on the monster's body; weapon impact sound unchanged.
2. Hit with a weapon without the field (or unarmed) → `DefaultHitVFX` appears; fallback sound (`SoundToPlay`) unchanged.
3. Get hit by a monster's melee attack → `DefaultHitVFX` appears at the impact point (only if the monster's attack GA has `GameplayCueTag` set — otherwise unchanged, which is acceptable).
4. Rapid combo attacks → impact VFX plays per hit with no accumulation/leak (one-shot Niagara auto-destroys).
5. Sound double-check: weapon sound plays exactly once per hit (if it plays twice, the BP graph logic from Step 1.2 wasn't removed).

- [ ] **Step 3: Fix anything the checklist surfaces, then done**

No commit here unless code fixes were needed; .uasset saves are committed by the user's normal flow.
