# Blade Wave Skill Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Hold-to-charge skill that fires a piercing sword-beam projectile toward the mouse cursor on release, with damage/size scaling by charge time (0.5s–3s).

**Architecture:** New GAS ability (`UR1GameplayAbility_BladeWave`) using the project's existing montage+gameplay-event boilerplate; key-release input routed as slot-tagged gameplay events from `AR1PlayerController`; new piercing projectile subclass of `AR1Projectile`. Cost is paid and cooldown starts only at fire time (CommitAbility deferred to the fire anim-notify).

**Tech Stack:** UE 5.3, C++ (module `R1`), GAS, Enhanced Input. No automated test framework exists in this repo — each code task ends with a full editor-target build; behavior verification is the manual PIE checklist in Task 5.

**Spec:** `docs/superpowers/specs/2026-07-09-blade-wave-design.md`

## Global Constraints

- Build command (editor must be CLOSED first — every task here touches headers, Live Coding cannot apply them):
  ```powershell
  Get-Process UnrealEditor -ErrorAction SilentlyContinue   # if this returns a process, ASK THE USER to close the editor; do not kill it
  & "C:\Program Files\Epic Games\UE_5.3\Engine\Build\BatchFiles\Build.bat" R1Editor Win64 Development "C:\Users\owner\Documents\GitHub\R1\R1.uproject" -waitmutex
  ```
  Success = output ends with `Total execution time`, exit code 0. Incremental builds ≈ 20–30s.
- Epic coding standard: `U`/`A`/`F` prefixes, `b` bool prefix, PascalCase verbs. UObject members = raw pointer + `UPROPERTY()` (never std smart pointers). `UE_LOG(LogR1, ...)`, never printf.
- Singleplayer project: no new `HasAuthority()` branches, no new RPCs, no new replication.
- AttributeSet values only change via GameplayEffects. Never `Set` directly.
- Comments in this codebase are mostly Korean — match the surrounding style, and only comment non-obvious constraints.
- Do NOT edit `.uasset`/`.umap` files or anything in `Binaries/`, `Intermediate/`, `Saved/`.
- `*.generated.h` is always the LAST include in a header.
- Commit messages: `feat: ...` style, ending with `Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>`.
- Runtime ASC note: for the player, `AR1Player::InitAbilitySystem()` (`Source/R1/Character/R1Player.cpp:117`) points the character's ASC at the **PlayerState's ASC** — abilities, montage gameplay events, and the Mana attribute (`UPlayerAttributeSet`) all live on that one component. `GetAbilitySystemComponentFromActorInfo()` inside an ability returns it; no cross-ASC routing is needed.

---

### Task 1: Gameplay tags + slot→release-tag helper

**Files:**
- Modify: `Source/R1/R1GameplayTags.h`
- Modify: `Source/R1/R1GameplayTags.cpp`

**Interfaces:**
- Consumes: `ER1SkillSlot` from `Source/R1/R1Define.h` (values: `None, Q, W, E, R`)
- Produces (used by Tasks 2 and 4):
  - Tags: `R1GameplayTags::Event_Skill_Release_Q/W/E/R`, `Event_Montage_BladeWave`, `Data_Skill_Cooldown`, `Cooldown_Skill_BladeWave`
  - `FGameplayTag R1GameplayTags::GetSkillReleaseTag(ER1SkillSlot Slot)` — returns the matching release tag, empty tag for `None`

- [ ] **Step 1: Add tag declarations to `R1GameplayTags.h`**

Add a forward declaration of the enum above the namespace (the header must NOT include `R1Define.h`), and the new declarations inside the namespace after the existing `Event_Hit_Critical` line (`R1GameplayTags.h:25`):

```cpp
// (namespace 위, #include 아래에 추가)
enum class ER1SkillSlot : uint8;
```

```cpp
	// 스킬 키 릴리즈(홀드 해제) 이벤트 — 홀드형 스킬(블레이드 웨이브 등)이 슬롯별로 대기
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Skill_Release_Q);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Skill_Release_W);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Skill_Release_E);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Skill_Release_R);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_BladeWave);

	// 슬롯(Q/W/E/R)에 해당하는 릴리즈 이벤트 태그. None이면 빈 태그.
	FGameplayTag GetSkillReleaseTag(ER1SkillSlot Slot);
```

And next to the existing `Data_Skill_Cost` declaration (`R1GameplayTags.h:55`):

```cpp
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Skill_Cooldown);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Skill_BladeWave);
```

- [ ] **Step 2: Add definitions to `R1GameplayTags.cpp`**

Add `#include "R1Define.h"` to the top of the file, then the definitions inside the namespace (near the other `Event_` defines; note the existing `Event_Montage_Attack` string has a legacy typo "Monage" — do NOT copy that typo, new tags use correct spelling):

```cpp
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_Release_Q, "Event.Skill.Release.Q");
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_Release_W, "Event.Skill.Release.W");
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_Release_E, "Event.Skill.Release.E");
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_Release_R, "Event.Skill.Release.R");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_BladeWave, "Event.Montage.BladeWave");
	UE_DEFINE_GAMEPLAY_TAG(Data_Skill_Cooldown, "Data.Skill.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_BladeWave, "Cooldown.Skill.BladeWave");

	FGameplayTag GetSkillReleaseTag(ER1SkillSlot Slot)
	{
		switch (Slot)
		{
		case ER1SkillSlot::Q: return Event_Skill_Release_Q;
		case ER1SkillSlot::W: return Event_Skill_Release_W;
		case ER1SkillSlot::E: return Event_Skill_Release_E;
		case ER1SkillSlot::R: return Event_Skill_Release_R;
		default:              return FGameplayTag();
		}
	}
```

- [ ] **Step 3: Build**

Run the Global Constraints build command. Expected: exit 0, `Total execution time` in output.

- [ ] **Step 4: Commit**

```powershell
git add Source/R1/R1GameplayTags.h Source/R1/R1GameplayTags.cpp
git commit -m @'
feat: add skill-release and Blade Wave gameplay tags

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>
'@
```

---

### Task 2: Release-input routing (controller) + slot lookup (equipment manager)

**Files:**
- Modify: `Source/R1/Player/R1PlayerController.h` (private handler section, near `ExecuteSkill` at line 47)
- Modify: `Source/R1/Player/R1PlayerController.cpp` (`SetupInputComponent` bindings ~lines 121–135; handlers near `ExecuteSkill` ~line 587)
- Modify: `Source/R1/System/R1EquipmentManagerComponent.h` (public section near `ExecuteSkillSlot` at line 50)
- Modify: `Source/R1/System/R1EquipmentManagerComponent.cpp` (near `ExecuteSkillSlot` at line 305)

**Interfaces:**
- Consumes: `R1GameplayTags::GetSkillReleaseTag(ER1SkillSlot)` from Task 1
- Produces (used by Task 4):
  - On any skill-key release, a gameplay event with tag `Event.Skill.Release.<Slot>` is sent to the player character actor (payload `Instigator` = player)
  - `ER1SkillSlot UR1EquipmentManagerComponent::GetSlotForHandle(FGameplayAbilitySpecHandle AbilityHandle) const` — returns `ER1SkillSlot::None` if not slotted

- [ ] **Step 1: Declare handlers in `R1PlayerController.h`**

In the private section, directly below `void ExecuteSkill(ER1SkillSlot Slot);` (line 47):

```cpp
	void OnQSkillReleased();
	void OnWSkillReleased();
	void OnESkillReleased();
	void OnRSkillReleased();

	// 스킬 키를 뗐을 때 홀드형 스킬(차지)에게 릴리즈 이벤트를 전달한다.
	void ReleaseSkill(ER1SkillSlot Slot);
```

- [ ] **Step 2: Bind release events in `SetupInputComponent`**

In `R1PlayerController.cpp`, after each existing skill `Started` binding (lines 121–135), add `Completed` + `Canceled` bindings. Example for Q (repeat the same two lines for W/E/R with the matching handler):

```cpp
	EnhancedInputComponent->BindAction(ActionQSkill, ETriggerEvent::Started, this, &ThisClass::OnQSkill);
	EnhancedInputComponent->BindAction(ActionQSkill, ETriggerEvent::Completed, this, &ThisClass::OnQSkillReleased);
	EnhancedInputComponent->BindAction(ActionQSkill, ETriggerEvent::Canceled, this, &ThisClass::OnQSkillReleased);
```

- [ ] **Step 3: Implement handlers**

In `R1PlayerController.cpp`, below `ExecuteSkill` (line 593). Ensure `#include "AbilitySystemBlueprintLibrary.h"` is present in the include list (add if missing; `R1GameplayTags.h` is already included):

```cpp
void AR1PlayerController::OnQSkillReleased() { ReleaseSkill(ER1SkillSlot::Q); }
void AR1PlayerController::OnWSkillReleased() { ReleaseSkill(ER1SkillSlot::W); }
void AR1PlayerController::OnESkillReleased() { ReleaseSkill(ER1SkillSlot::E); }
void AR1PlayerController::OnRSkillReleased() { ReleaseSkill(ER1SkillSlot::R); }

void AR1PlayerController::ReleaseSkill(ER1SkillSlot Slot)
{
	// 주의: IsCasting() 가드를 걸지 않는다 — 차지(Casting) 중인 어빌리티가 이 이벤트를 받아야 발사된다.
	if (R1Player == nullptr)
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.Instigator = R1Player;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(R1Player, R1GameplayTags::GetSkillReleaseTag(Slot), Payload);
}
```

Charging abilities are the only listeners; when nothing is charging the event is a no-op.

- [ ] **Step 4: Add `GetSlotForHandle` to the equipment manager**

`R1EquipmentManagerComponent.h`, public section below `AssignSkillToSlot` (line 54):

```cpp
	// 해당 어빌리티 핸들이 꽂혀 있는 스킬 슬롯을 반환 (없으면 None)
	ER1SkillSlot GetSlotForHandle(FGameplayAbilitySpecHandle AbilityHandle) const;
```

`R1EquipmentManagerComponent.cpp`, below `ExecuteSkillSlot` (line 317):

```cpp
ER1SkillSlot UR1EquipmentManagerComponent::GetSlotForHandle(FGameplayAbilitySpecHandle AbilityHandle) const
{
	for (const TPair<ER1SkillSlot, FGameplayAbilitySpecHandle>& Pair : SkillSlotsMap)
	{
		if (Pair.Value == AbilityHandle)
		{
			return Pair.Key;
		}
	}
	return ER1SkillSlot::None;
}
```

- [ ] **Step 5: Build**

Run the Global Constraints build command. Expected: exit 0.

- [ ] **Step 6: Commit**

```powershell
git add Source/R1/Player/R1PlayerController.h Source/R1/Player/R1PlayerController.cpp Source/R1/System/R1EquipmentManagerComponent.h Source/R1/System/R1EquipmentManagerComponent.cpp
git commit -m @'
feat: route skill-key release as slot-tagged gameplay events

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>
'@
```

---

### Task 3: Piercing projectile — `AR1BladeWaveProjectile`

**Files:**
- Modify: `Source/R1/Object/R1Projectile.h` (line 36: make `OnOverlap` virtual)
- Create: `Source/R1/Object/R1BladeWaveProjectile.h`
- Create: `Source/R1/Object/R1BladeWaveProjectile.cpp`

**Interfaces:**
- Consumes: `AR1Projectile` (base: `SphereComponent`, `ProjectileMovement`, public `DamageSpecHandle`, `BeginPlay` binds `OnOverlap` and sets 3s lifespan)
- Produces (used by Task 4):
  - `AR1BladeWaveProjectile` — pierces: never destroys on pawn overlap, damages each `AR1Character` once, skips instigator and dead targets
  - `void SetChargeScale(float InScale)` — uniform actor scale (collision + visuals); call once right after spawn
  - Caller is expected to set `ProjectileMovement->MaxSpeed`, `ProjectileMovement->Velocity`, `SetLifeSpan()`, and `DamageSpecHandle` after spawning

- [ ] **Step 1: Make the base overlap handler virtual**

`R1Projectile.h` line 35–36 — change:

```cpp
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
```

to:

```cpp
	UFUNCTION()
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
```

(The base `BeginPlay` `AddDynamic` binding resolves the UFunction by name and dispatches virtually — the subclass override runs without rebinding. The override must NOT redeclare `UFUNCTION()`.)

- [ ] **Step 2: Create `R1BladeWaveProjectile.h`**

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Object/R1Projectile.h"
#include "R1BladeWaveProjectile.generated.h"

/**
 * 블레이드 웨이브 검기 투사체: 파괴되지 않고 관통하며, 캐릭터마다 1회씩만 피해를 준다.
 * 속도/수명/데미지 스펙은 스폰 직후 어빌리티가 세팅한다.
 */
UCLASS()
class R1_API AR1BladeWaveProjectile : public AR1Projectile
{
	GENERATED_BODY()

public:
	// 차지 비율에 따른 균등 스케일 (충돌 구체 + 비주얼). 스폰 직후 1회 호출.
	void SetChargeScale(float InScale);

protected:
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	// 관통 중복 타격 방지용 기록
	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActors;
};
```

- [ ] **Step 3: Create `R1BladeWaveProjectile.cpp`**

```cpp
#include "Object/R1BladeWaveProjectile.h"
#include "Character/R1Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

void AR1BladeWaveProjectile::SetChargeScale(float InScale)
{
	SetActorScale3D(FVector(InScale));
}

void AR1BladeWaveProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 베이스와 달리 Destroy하지 않는다 — 관통하며 대상마다 1회만 피해 적용.
	if (OtherActor == nullptr || OtherActor == GetInstigator() || HitActors.Contains(OtherActor))
	{
		return;
	}

	AR1Character* TargetCharacter = Cast<AR1Character>(OtherActor);
	if (TargetCharacter == nullptr || TargetCharacter->GetCreatureState() == ECreatureState::Dead)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (TargetASC == nullptr || DamageSpecHandle.IsValid() == false)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	if (SourceASC == nullptr)
	{
		return;
	}

	HitActors.Add(OtherActor);
	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
}
```

(`ECreatureState` comes through `R1Character.h`'s includes, same as `R1Projectile.cpp`'s sibling files; add `#include "R1Define.h"` if the build complains.)

- [ ] **Step 4: Build**

Run the Global Constraints build command. Expected: exit 0.

- [ ] **Step 5: Commit**

```powershell
git add Source/R1/Object/R1Projectile.h Source/R1/Object/R1BladeWaveProjectile.h Source/R1/Object/R1BladeWaveProjectile.cpp
git commit -m @'
feat: add piercing Blade Wave projectile actor

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>
'@
```

---

### Task 4: The ability — `UR1GameplayAbility_BladeWave` + HitReact hyper-armor block

**Files:**
- Create: `Source/R1/AbilitySystem/Abilities/R1GameplayAbility_BladeWave.h`
- Create: `Source/R1/AbilitySystem/Abilities/R1GameplayAbility_BladeWave.cpp`
- Modify: `Source/R1/AbilitySystem/Abilities/R1GameplayAbility_HitReact.cpp` (constructor, line 12–21)

**Interfaces:**
- Consumes:
  - `UR1GameplayAbility::PlayAttackMontageAndWaitForEvent(AR1Character*, const FGameplayTag&, FName)` — sets Casting state, plays `MontageToPlay` at AttackSpeed rate, fires `OnAttackEventReceived(FGameplayEventData)` when the montage notify sends the tag. NOTE: it plays with `bStopWhenAbilityEnds=false`, so cancelled endings MUST call `MontageStop()` or the charge loop keeps playing.
  - `R1GameplayTags::GetSkillReleaseTag(ER1SkillSlot)`, `Event_Montage_BladeWave`, `Data_Skill_Cooldown`, `Character_State_UnInterruptable` (Task 1; UnInterruptable already existed)
  - `UR1EquipmentManagerComponent::GetSlotForHandle(...)` (Task 2)
  - `AR1BladeWaveProjectile::SetChargeScale(float)` + base members (Task 3)
  - `UR1GameInstance::GetSkillData(FName)` → `FSkillDataRow { Damage, ManaCost, Cooldown, Range }`
  - `AR1Player::GetEquipmentComponent()`, `AR1PlayerController::GetHitResultUnderCursor(ECC_GameTraceChannel2, ...)`
- Produces: the complete skill; designers configure a BP subclass (Task 5) setting `MontageToPlay`, `AttackEventTag`, `DamageEffect`, `ProjectileClass`, `CostGameplayEffectClass`, `CooldownGameplayEffectClass`.

- [ ] **Step 1: Create `R1GameplayAbility_BladeWave.h`**

```cpp
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "R1Define.h"
#include "R1GameplayAbility_BladeWave.generated.h"

class AR1BladeWaveProjectile;
class UGameplayEffect;

/**
 * 블레이드 웨이브: 스킬 키를 누르고 있는 동안 차지, 떼면 커서 방향으로 관통 검기 발사.
 * - 차지 0.5s(최소)~3s(최대), 최대 차지 상태에서는 키를 뗄 때까지 유지
 * - 데미지·크기가 차지 비율로 스케일
 * - 마나/쿨다운은 발사 시점에만 지불 (조기 취소는 무료)
 * - 차지 중 피격 리액션 차단 (하이퍼아머, 데미지는 그대로 받음)
 */
UCLASS()
class R1_API UR1GameplayAbility_BladeWave : public UR1GameplayAbility
{
	GENERATED_BODY()

public:
	UR1GameplayAbility_BladeWave(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual void OnMontageEnded() override;
	// 발사 노티파이(Event.Montage.BladeWave) 수신 → 투사체 스폰 + 비용/쿨다운 커밋
	virtual void OnAttackEventReceived(FGameplayEventData Payload) override;

	UFUNCTION()
	void OnSkillKeyReleased(FGameplayEventData Payload);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Data")
	FName SkillID = FName("BladeWave");

	UPROPERTY(EditAnywhere, Category = "BladeWave")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere, Category = "BladeWave")
	TSubclassOf<AR1BladeWaveProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "BladeWave")
	float ProjectileSpeed = 1200.0f;

	// [설정] 애니메이션에서 보낼 발사 이벤트 태그 (Event.Montage.BladeWave)
	UPROPERTY(EditDefaultsOnly, Category = "BladeWave")
	FGameplayTag AttackEventTag;

	UPROPERTY(EditAnywhere, Category = "BladeWave|Charge")
	float MinChargeTime = 0.5f;

	UPROPERTY(EditAnywhere, Category = "BladeWave|Charge")
	float MaxChargeTime = 3.0f;

	UPROPERTY(EditAnywhere, Category = "BladeWave|Charge")
	float MinDamageScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "BladeWave|Charge")
	float MaxDamageScale = 2.5f;

	UPROPERTY(EditAnywhere, Category = "BladeWave|Charge")
	float MinSizeScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "BladeWave|Charge")
	float MaxSizeScale = 2.0f;

	// 몽타주의 발사 섹션 이름
	UPROPERTY(EditAnywhere, Category = "Animation")
	FName FireSectionName = FName("Fire");

private:
	float ChargeStartTime = 0.0f;
	float CachedDamageScale = 1.0f;
	float CachedSizeScale = 1.0f;
	FVector CachedFireDirection = FVector::ForwardVector;
	bool bReleased = false;

	// DT_SkillData에서 OnAvatarSet 시점에 캐싱
	float CachedSkillDamage = 0.0f;
	float CachedManaCost = 0.0f;
	float CachedCooldown = 0.0f;
	float CachedRange = 0.0f;
};
```

- [ ] **Step 2: Create `R1GameplayAbility_BladeWave.cpp`**

```cpp
#include "AbilitySystem/Abilities/R1GameplayAbility_BladeWave.h"
#include "R1LogChannels.h"
#include "R1GameplayTags.h"
#include "Character/R1Player.h"
#include "Player/R1PlayerController.h"
#include "System/R1GameInstance.h"
#include "System/R1EquipmentManagerComponent.h"
#include "Object/R1BladeWaveProjectile.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UR1GameplayAbility_BladeWave::UR1GameplayAbility_BladeWave(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	SkillType = ER1SkillType::Active;

	// 차지 중 피격 리액션 차단 (HitReact의 ActivationBlockedTags와 짝)
	ActivationOwnedTags.AddTag(R1GameplayTags::Character_State_UnInterruptable);
}

void UR1GameplayAbility_BladeWave::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AR1Player* Player = Cast<AR1Player>(ActorInfo->AvatarActor.Get());
	if (Player == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 어느 슬롯(Q/W/E/R)에 꽂혀 있는지 확인해 그 키의 릴리즈 이벤트만 기다린다.
	ER1SkillSlot MySlot = ER1SkillSlot::None;
	if (UR1EquipmentManagerComponent* EquipComp = Player->GetEquipmentComponent())
	{
		MySlot = EquipComp->GetSlotForHandle(Handle);
	}
	if (MySlot == ER1SkillSlot::None)
	{
		UE_LOG(LogR1, Warning, TEXT("[BladeWave] 스킬 슬롯을 찾지 못해 시전을 취소합니다."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 차지 몽타주 재생(Charge → ChargeLoop 루프) + 발사 노티파이 대기
	if (PlayAttackMontageAndWaitForEvent(Player, AttackEventTag) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bReleased = false;
	ChargeStartTime = GetWorld()->GetTimeSeconds();

	UAbilityTask_WaitGameplayEvent* WaitReleaseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		R1GameplayTags::GetSkillReleaseTag(MySlot),
		nullptr,
		true,	// OnlyTriggerOnce
		true	// OnlyMatchExact
	);
	WaitReleaseTask->EventReceived.AddDynamic(this, &UR1GameplayAbility_BladeWave::OnSkillKeyReleased);
	WaitReleaseTask->ReadyForActivation();
}

void UR1GameplayAbility_BladeWave::OnSkillKeyReleased(FGameplayEventData Payload)
{
	if (bReleased)
	{
		return;
	}

	const float Elapsed = GetWorld()->GetTimeSeconds() - ChargeStartTime;

	// 최소 차지 미달 → 무료 취소 (마나/쿨다운 없음)
	if (Elapsed < MinChargeTime)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	bReleased = true;

	const float Denominator = FMath::Max(MaxChargeTime - MinChargeTime, KINDA_SMALL_NUMBER);
	const float ChargeRatio = FMath::Clamp((Elapsed - MinChargeTime) / Denominator, 0.0f, 1.0f);
	CachedDamageScale = FMath::Lerp(MinDamageScale, MaxDamageScale, ChargeRatio);
	CachedSizeScale = FMath::Lerp(MinSizeScale, MaxSizeScale, ChargeRatio);

	AR1Player* Player = Cast<AR1Player>(GetAvatarActorFromActorInfo());
	if (Player == nullptr)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 릴리즈 순간의 커서 위치로 캐릭터를 회전시키고 발사 방향을 확정한다.
	CachedFireDirection = Player->GetActorForwardVector();
	if (AR1PlayerController* PC = Cast<AR1PlayerController>(Player->GetController()))
	{
		FHitResult CursorHit;
		if (PC->GetHitResultUnderCursor(ECC_GameTraceChannel2, false, CursorHit))
		{
			FVector Direction = CursorHit.Location - Player->GetActorLocation();
			Direction.Z = 0.0f;
			if (Direction.Normalize())
			{
				CachedFireDirection = Direction;
				Player->SetActorRotation(Direction.Rotation());
			}
		}
	}
	CachedFireDirection.Z = 0.0f;
	CachedFireDirection = CachedFireDirection.GetSafeNormal();

	// 발사 섹션으로 점프 → 섹션 내 노티파이가 OnAttackEventReceived를 호출한다.
	MontageJumpToSection(FireSectionName);
}

void UR1GameplayAbility_BladeWave::OnAttackEventReceived(FGameplayEventData Payload)
{
	// 발사 확정 전(차지 중) 노티파이 수신은 무시 — 몽타주 구성 실수 안전망
	if (bReleased == false)
	{
		return;
	}

	// 이 시점에 마나 차감 + 쿨다운 시작 (릴리즈 전 취소는 비용 없음)
	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo) == false)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	AR1Player* Player = Cast<AR1Player>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (Player == nullptr || SourceASC == nullptr || ProjectileClass == nullptr || DamageEffect == nullptr)
	{
		UE_LOG(LogR1, Error, TEXT("[BladeWave] 발사 실패 — ProjectileClass/DamageEffect 설정을 확인하세요."));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const FVector SpawnLocation = Player->GetActorLocation() + CachedFireDirection * 100.0f;
	const FRotator SpawnRotation = CachedFireDirection.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Player;
	SpawnParams.Instigator = Player;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AR1BladeWaveProjectile* Projectile = GetWorld()->SpawnActor<AR1BladeWaveProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (Projectile)
	{
		Projectile->ProjectileMovement->MaxSpeed = ProjectileSpeed;
		Projectile->ProjectileMovement->Velocity = CachedFireDirection * ProjectileSpeed;
		Projectile->SetChargeScale(CachedSizeScale);

		if (CachedRange > 0.0f && ProjectileSpeed > 0.0f)
		{
			Projectile->SetLifeSpan(CachedRange / ProjectileSpeed);
		}

		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Magnitude, CachedSkillDamage * CachedDamageScale);
			Projectile->DamageSpecHandle = SpecHandle;
		}
	}

	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, Player->GetActorLocation());
	}
}

void UR1GameplayAbility_BladeWave::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UR1GameplayAbility_BladeWave::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 취소 종료 시 차지 루프 몽타주를 직접 정지해야 한다.
	// (PlayAttackMontageAndWaitForEvent가 bStopWhenAbilityEnds=false로 재생하므로 자동으로 멈추지 않음)
	if (bWasCancelled)
	{
		MontageStop();
	}

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (AR1Player* Player = Cast<AR1Player>(ActorInfo->AvatarActor.Get()))
		{
			if (Player->GetCreatureState() != ECreatureState::Dead)
			{
				Player->SetCreatureState(ECreatureState::Idle);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UR1GameplayAbility_BladeWave::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid())
	{
		return;
	}

	UWorld* World = ActorInfo->AvatarActor->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (UR1GameInstance* GameInstance = Cast<UR1GameInstance>(World->GetGameInstance()))
	{
		if (const FSkillDataRow* SkillData = GameInstance->GetSkillData(SkillID))
		{
			CachedSkillDamage = SkillData->Damage;
			CachedManaCost = SkillData->ManaCost;
			CachedCooldown = SkillData->Cooldown;
			CachedRange = SkillData->Range;
		}
		else
		{
			UE_LOG(LogR1, Error, TEXT("[BladeWave] SkillID '%s'에 해당하는 스킬 데이터를 찾을 수 없습니다!"), *SkillID.ToString());
		}
	}
}

bool UR1GameplayAbility_BladeWave::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (CachedManaCost <= 0.0f)
	{
		return true;
	}

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		const float CurrentMana = ASC->GetNumericAttribute(UPlayerAttributeSet::GetManaAttribute());
		if (CurrentMana < CachedManaCost)
		{
			UE_LOG(LogR1, Warning, TEXT("[BladeWave] 마나 부족 — 필요: %f, 현재: %f"), CachedManaCost, CurrentMana);
			return false;
		}
	}
	return true;
}

void UR1GameplayAbility_BladeWave::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (CostGameplayEffectClass && CachedManaCost > 0.0f)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CostGameplayEffectClass, GetAbilityLevel(Handle, ActorInfo));
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Cost, -CachedManaCost);
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}
}

void UR1GameplayAbility_BladeWave::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	// 쿨다운 GE의 지속시간을 DT_SkillData의 Cooldown 값으로 주입한다. (Super 미호출 — GE 고정 duration 대체)
	if (CooldownGameplayEffectClass && CachedCooldown > 0.0f)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CooldownGameplayEffectClass, GetAbilityLevel(Handle, ActorInfo));
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Cooldown, CachedCooldown);
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}
}
```

- [ ] **Step 3: Block HitReact during hyper-armor**

`R1GameplayAbility_HitReact.cpp` constructor (line 12–21) — add one line after the existing `ActivationOwnedTags.AddTag(...)`:

```cpp
	// 차지형 스킬(하이퍼아머) 시전 중에는 피격 리액션을 차단한다.
	ActivationBlockedTags.AddTag(R1GameplayTags::Character_State_UnInterruptable);
```

- [ ] **Step 4: Build**

Run the Global Constraints build command. Expected: exit 0.

- [ ] **Step 5: Commit**

```powershell
git add Source/R1/AbilitySystem/Abilities/R1GameplayAbility_BladeWave.h Source/R1/AbilitySystem/Abilities/R1GameplayAbility_BladeWave.cpp Source/R1/AbilitySystem/Abilities/R1GameplayAbility_HitReact.cpp
git commit -m @'
feat: add Blade Wave hold-to-charge ability with hyper-armor

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>
'@
```

---

### Task 5: Editor content + PIE verification (user-driven, no code)

**Files:** none (all in-editor). Claude's role in this task: walk the user through it and record results.

**Editor checklist (user performs):**

- [ ] **`AM_BladeWave`**: create montage sections `Charge` (start) → `ChargeLoop` → `Fire`. In the Montage panel set section linking: `Charge` → next `ChargeLoop`; `ChargeLoop` → next `ChargeLoop` (loops while held); `Fire` must NOT be linked from `ChargeLoop`. Add the same gameplay-event anim notify class used by existing attack montages (the one that sends `Event.Monage.Attack`) at the swing frame inside `Fire`, with tag **`Event.Montage.BladeWave`**.
- [ ] **`BP_BladeWaveProjectile`** (suggested: `Content/Blueprints/Weapons/` or wherever projectile BPs live): parent = `R1BladeWaveProjectile`. Add beam VFX (Niagara/mesh) under the sphere. Keep the `MonsterProjectile` collision profile on the sphere (instigator check prevents self-hits).
- [ ] **`GE_Cooldown_BladeWave`**: Duration Policy = Has Duration; Duration Magnitude = SetByCaller with tag `Data.Skill.Cooldown`; grant tag `Cooldown.Skill.BladeWave` to the target (UE 5.3: add the *Target Tags* Gameplay Effect Component / "Grant Tags to Target Actor").
- [ ] **`GA_BladeWave`**: BP subclass of `R1GameplayAbility_BladeWave`. Set: `MontageToPlay = AM_BladeWave`, `AttackEventTag = Event.Montage.BladeWave`, `DamageEffect` = the same damage GE existing attack abilities use (with `UR1DamageExecutionCalc` + `Data.Skill.Magnitude`), `ProjectileClass = BP_BladeWaveProjectile`, `CostGameplayEffectClass` = the mana-cost GE JumpAttack's BP uses (`Data.Skill.Cost`), `CooldownGameplayEffectClass = GE_Cooldown_BladeWave`. Tune scale ranges if desired.
- [ ] **`DT_SkillData`**: add row `BladeWave` — suggested starting values: Damage 40, ManaCost 25, Cooldown 8, Range 1500.
- [ ] **Weapon DataAsset**: add `GA_BladeWave` to `GrantedAbilities` of a test weapon (e.g. one of the GreatSword DAs). SkillType is already Active in C++, so it auto-assigns to the first free slot on equip.

**PIE verification checklist (from the spec):**

- [ ] Equip the weapon → log shows the skill auto-assigned to a slot.
- [ ] Tap the key (<0.5s) → charge anim cancels, no beam, no mana change, no cooldown.
- [ ] Hold ~1s → release: character snaps to cursor, beam fires, mana deducted once, cooldown starts.
- [ ] Hold past 3s → character stays in charge loop until release; released beam is visibly bigger and deals max damage (check damage numbers vs. a short-charge beam).
- [ ] Beam pierces a pack: every monster damaged exactly once, beam continues to max range.
- [ ] During charge, get hit → no hit-react, charge continues, HP drops.
- [ ] During charge, press/release a *different* skill key → charge unaffected.
- [ ] Cooldown: re-press immediately after firing → ability does not activate until cooldown expires.
- [ ] Open inventory mid-charge, release the key → verify no stuck charge loop (if the release event is swallowed by UI input mode, note it and report — known risk from the spec).
- [ ] Die mid-charge → no stuck montage/state after respawn flow.

---

## Self-review notes

- Spec coverage: grant/auto-slot (Task 5 DA), hold/release plumbing (Tasks 1–2), charge model + scaling + cursor aim + rooted state (Task 4), free early-cancel + pay-on-fire (Task 4 CommitAbility placement), hyper-armor (Task 4 Step 3), piercing hit-once projectile + range lifespan (Tasks 3–4), content + tests (Task 5). Cooldown open item resolved: SetByCaller-duration cooldown GE via `ApplyCooldown` override.
- Deviation from spec (improvement): reuse existing unused `Character.State.UnInterruptable` tag instead of adding `Character.State.Charging`.
- Known repo quirk: `PlayAttackMontageAndWaitForEvent` uses `bStopWhenAbilityEnds=false` → `EndAbility` explicitly calls `MontageStop()` on cancel to kill the charge loop.
