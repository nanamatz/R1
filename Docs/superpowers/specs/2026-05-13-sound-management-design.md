# Sound Management Design: Minion Attack & HitReact

## Goal
Implement direct sound playback for Minion Melee Attack and HitReact abilities, ensuring audio is synchronized with animation events and ability activation.

## Architecture
We will use the existing `SoundToPlay` property in `UR1GameplayAbility` and `UGameplayStatics::PlaySoundAtLocation` for spatialized audio playback.

### 1. Minion Melee Attack (`UR1GameplayAbility_MonsterMeeleAttack`)
- **Location:** `OnAttackEventReceived`
- **Timing:** Triggered by `GameplayEvent` Notify in the attack montage.
- **Logic:**
  ```cpp
  if (SoundToPlay)
  {
      UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, SourceCharacter->GetActorLocation());
  }
  ```

### 2. Hit React (`UR1GameplayAbility_HitReact`)
- **Location:** `ActivateAbility`
- **Timing:** Triggered immediately upon ability activation (when hit occurs).
- **Logic:**
  ```cpp
  if (SoundToPlay)
  {
      UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, Monster->GetActorLocation());
  }
  ```

## Tagging & Interaction
- **Ability Tag:** `Ability.Monster.Attack` for the attack ability.
- **Interruption:** HitReact ability should cancel abilities with the `Ability.Monster.Attack` tag to ensure responsiveness.

## Verification Plan
1. **Minion Attack:**
   - Assign a sound to a Minion's attack ability BP.
   - Observe the minion attacking.
   - Confirm sound plays exactly when the `r1sendgameplayevent` notify is reached.
2. **Hit React:**
   - Assign a hit sound to the HitReact ability BP.
   - Attack a minion.
   - Confirm sound plays immediately when the minion flinches.
