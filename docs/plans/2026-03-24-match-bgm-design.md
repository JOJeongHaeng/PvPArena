# Match BGM Design

**Goal:** Play `Starter_Music_Cue` for all non-combat match phases and `Starter_Background_Cue` during gameplay, while preserving playback position when transitioning between non-combat phases like `Lobby` and `MatchEnd`.

**Recommended Approach:** Manage local-only BGM in `UPvPArenaHUDWidget`, which already polls `APvPArenaGameState` and updates phase-aware UI. Compute the target music from `EPvPAMatchPhase`, switch tracks only when the desired asset actually changes, and keep the current track playing when phase transitions still resolve to the same music cue.

## Audio Model

- `Lobby` uses `/Game/PvPArena/Audio/Starter_Music_Cue`.
- `MatchEnd` uses `/Game/PvPArena/Audio/Starter_Music_Cue`.
- `Playing` uses `/Game/PvPArena/Audio/Starter_Background_Cue`.
- Non-combat to non-combat transitions do not stop or restart `Starter_Music_Cue`.

## Implementation Model

- Add one local `UAudioComponent` to the HUD widget for background music.
- Add default music asset properties for non-combat and gameplay music.
- Add a small helper that maps `EPvPAMatchPhase` to the desired asset path or asset pointer.
- Refresh music during the existing `RefreshWidgetData()` polling path.
- Only call `Stop`/`Play` when the requested music asset differs from the currently assigned one.

## Testing

- Verify HUD widget exposes the two default BGM properties.
- Verify the asset paths point at `Starter_Music_Cue` and `Starter_Background_Cue`.
- Verify the phase-to-music helper returns lobby music for `Lobby` and `MatchEnd`, and gameplay music for `Playing`.
