# Per-Floor Ambient Sound — Design

**Date:** 2026-07-11
**Branch:** feat-game-music
**Scope:** Editor-only (content). No C++ changes.

## Goal

Give each dungeon floor (1F / 2F / 3F) a distinct atmosphere by playing a
floor-specific looping ambient track that is audible across the entire floor.

## Decisions

- **Audibility:** whole floor, non-spatialized. The actor lives in the floor's
  start-room level purely as a convenient home; the sound is 2D and uniform
  everywhere on the floor.
- **Volume routing:** route to the existing `BGMSoundClass` so the current
  BGM slider in the options menu controls it. No options-menu, save-game, or
  `R1SettingsSubsystem` changes.
- **Lifecycle:** rely on floor-at-once loading (`UR1RoomStreamingSubsystem`).
  Each floor's rooms are `ULevelStreamingDynamic` instances that unload on
  floor transition, so the ambient sound starts when the floor spawns and
  stops when it unloads. No cross-floor bleed is possible because only one
  floor is resident at a time.

## Implementation (per floor, x3)

1. **Sound asset** — one looping ambience track per floor.
   - Enable **Looping** on the wave (or loop inside a Sound Cue / MetaSound).
   - Set **Sound Class = BGM**.
   - Bake a 1–2 s fade-in into the asset (Sound Cue envelope or MetaSound) so
     the start is not abrupt when the floor finishes loading.
2. **Placement** — in that floor's start-room level asset, place one
   `AmbientSound` actor referencing the track.
   - **Auto Activate = on** (default).
   - **No attenuation asset assigned** and spatialization off, so the sound
     plays 2D at uniform volume across the whole floor.

## Transition behavior

Floor change hard-stops the old track (level unload) and hard-starts the new
one. The existing floor-transition camera fade masks the cut; the baked fade-in
covers the new track's onset. No crossfade system is added.

If crossfades or combat-music layering are wanted later, migrate to a small
code-driven ambience manager (WorldSubsystem or an `AR1MapGenerator`
floor-entry hook with a per-floor sound table). Out of scope here.

## Alternatives considered

- **Code-driven ambience manager** — enables crossfades, but adds C++ for a
  lifecycle the level streaming already provides. Rejected for now (YAGNI).
- **Spatialized sound at the start area only** — atmosphere would only be
  noticeable near the floor entrance. Rejected; goal is floor-wide identity.

## Verification

In PIE, per floor:

1. Ambience starts (with fade-in) after the floor loads and is equally audible
   in every room of the floor.
2. On floor transition, the old track stops and the new floor's track plays —
   no overlap of two tracks.
3. Lowering the BGM slider in the options menu lowers the ambience; the SFX
   slider does not affect it.
