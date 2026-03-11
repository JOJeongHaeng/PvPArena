# Random Respawn Starts Design

## Summary

This pass makes respawn start selection explicitly random when multiple `PlayerStart` actors are available, while avoiding an immediate repeat of the last-used spawn when alternatives exist. The goal is to make respawn variation intentional instead of relying on Unreal's default start selection behavior.

## Decisions

- Override `APvPArenaGameMode::ChoosePlayerStart`.
- When multiple valid `PlayerStart` actors exist, randomly choose from the set excluding the last-used start if possible.
- When only one valid start exists, reuse it.
- Keep the rest of the respawn flow unchanged.

## Rationale

Right now the project relies on `RestartPlayer()` and the engine's default player start choice. That means multiple `PlayerStart` actors may work, but the project does not explicitly guarantee random selection. By owning the selection rule in `APvPArenaGameMode`, the map setup becomes predictable: multiple starts mean randomized respawns, and obvious immediate repeats are avoided when there are alternatives.

## Implementation Scope

- Add a small helper that selects a spawn start from a provided candidate list while optionally excluding the previous start.
- Store the last chosen start in `APvPArenaGameMode`.
- Override `ChoosePlayerStart` to gather player starts and apply the helper.
- Add focused automation coverage for:
  - avoiding immediate repeats when more than one candidate exists
  - allowing reuse when only one candidate exists

## Non-Goals

- Do not add distance-based spawn weighting.
- Do not add team-based or opponent-aware spawn logic.
- Do not change respawn timing values.

## Verification

- Build the editor target.
- Run focused spawn-selection automation coverage.
- Manually confirm that multiple `PlayerStart` actors on the map now produce varied respawns.
