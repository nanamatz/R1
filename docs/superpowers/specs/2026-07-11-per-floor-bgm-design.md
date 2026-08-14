# Per-Floor Background Music — Design

**Date:** 2026-07-11 (retitled 2026-07-12: originally "ambient sound"; the
intent was always in-game BGM that changes per floor)
**Branch:** feat-game-music
**Scope:** Editor-only (content). No C++ changes.

## Goal

Give each dungeon floor (1F / 2F / 3F) its own looping background music track,
audible across the entire floor — one track per floor, including the boss
room. No combat/boss music switching (out of scope).

## Decisions

- **Audibility:** whole floor, non-spatialized (2D). The `AmbientSound` actor
  lives in the floor's start-room level purely as a convenient home.
- **Volume routing:** each track's **Sound Class must be set to
  `/Game/Blueprints/Audio/SC_BGM`** so the BGM slider (via
  `R1SettingsSubsystem`'s `SetSoundMixClassOverride`) controls it. A wave with
  no sound class bypasses the mix and ignores the slider.
- **Lifecycle:** rely on floor-at-once loading (`UR1RoomStreamingSubsystem`).
  Each floor's rooms are `ULevelStreamingDynamic` instances that unload on
  floor transition, so the music starts when the floor spawns and stops when
  it unloads. Only one floor is resident at a time, so tracks never overlap.

## Target assets (verified from CLI, 2026-07-12)

Start-room levels referenced by the room PDAs (`Content/Data/RoomDatas/Start/`):

| Floor | PDA | RoomLevel (place actor here) |
|-------|-----|------------------------------|
| 1F | `DA_Start_1F` | `/Game/Maps/1F/1F_S` |
| 2F | `DA_Start_2F` | `/Game/Maps/2F/2F_S` |
| 3F | `DA_Start_3F` | `/Game/Maps/3F/3F_Start` |

All three are classic gameplay maps (DungeonManager present).

> Note: `DA_Start_4F` and `DA_Start_5F` both reference `/Game/Maps/5F/5F_S` —
> floors 4 and 5 share one start-room level. If per-floor BGM is later
> extended to those floors, a level-placed sound cannot differ between them
> (and the 4F PDA pointing at a 5F map may itself be a mis-wire worth
> checking).

## Implementation (per floor, x3)

1. **Music asset** — one looping BGM track per floor.
   - Enable **Looping** on the wave (or loop inside a Sound Cue / MetaSound).
     Music loops must be seamless — prefer tracks authored to loop.
   - Set **Sound Class = SC_BGM**.
   - Optionally bake a 1–2 s fade-in (Sound Cue envelope or MetaSound) so the
     start is not abrupt when the floor finishes loading.
2. **Placement** — in that floor's start-room level, place one `AmbientSound`
   actor referencing the track.
   - **Auto Activate = on** (default).
   - **No attenuation asset assigned** and spatialization off, so the music
     plays 2D at uniform volume across the whole floor.

### Status as placed (2026-07-12, uncommitted)

- `1F_S`: AmbientSound placed → `SoundScape_v1_wav`
- `2F_S`: AmbientSound placed → `SoundScape_v1_wav` (**same track as 1F** —
  needs its own track for the per-floor identity goal)
- `3F_Start`: AmbientSound placed → `SoundScape_v2_wav`
- No attenuation on any of the three (correct).
- **Neither SoundScape wave has a Sound Class assigned** → BGM slider will
  not affect them until Sound Class = SC_BGM is set on each wave.

## Transition behavior

Floor change hard-stops the old track (level unload) and hard-starts the new
one. The existing floor-transition camera fade masks the cut. The track
restarts from the beginning whenever the floor loads (including save-game
load) — acceptable for looping BGM.

If crossfades or combat/boss music layering are wanted later, migrate to a
small code-driven music manager (WorldSubsystem or an `AR1MapGenerator`
floor-entry hook with a per-floor track table). Out of scope here.

## Alternatives considered

- **Code-driven music manager** — enables crossfades and combat layers, but
  adds C++ for a lifecycle the level streaming already provides. Rejected for
  now (YAGNI).
- **Spatialized sound at the start area only** — music would fade away from
  the entrance. Rejected; BGM must be floor-wide.

## Verification

In PIE, per floor:

1. Music starts after the floor loads and is equally audible in every room of
   the floor; it loops without an audible seam.
2. On floor transition, the old track stops and the new floor's track plays —
   no overlap of two tracks.
3. Lowering the BGM slider in the options menu lowers the music; the SFX
   slider does not affect it.
