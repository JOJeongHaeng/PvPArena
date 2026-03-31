# Default Crosshair During Match Design

## Goal

Keep the ranged crosshair visible during normal gameplay while preserving the existing charged ranged camera transition.

## Decision

Show the crosshair whenever the local match phase is `Playing`. Keep it hidden during `Lobby` and `MatchEnd`.

## Rationale

The crosshair visibility decision already lives in `UPvPArenaHUDWidget`, and the widget already reads match phase from `APvPArenaGameState`. The ranged camera transition is driven separately by `APvPArenaCharacter::bRangedChargeInputHeld`, so changing HUD visibility does not affect the charge camera blend.

## Expected Behavior

- Lobby: crosshair hidden
- Match playing, not charging: crosshair visible
- Match playing, charging: crosshair visible and camera still shifts into the ranged aim offset
- Match end: crosshair hidden

## Testing

Update the HUD crosshair automation test to assert visibility by match phase rather than only by charge input state.
