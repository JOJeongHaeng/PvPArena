# Round Start Unique Spawns Design

## Summary

Each new round should begin with players on different `PlayerStart` actors whenever multiple starts are available. This change is limited to round resets, not general mid-round respawns.

## Decisions

- Keep the current respawn selection logic for normal deaths.
- Add a round-start-only spawn selection path that excludes starts already assigned during the current round reset pass.
- Allow reuse only when the map has fewer unique `PlayerStart` actors than active players.

## Rationale

The current respawn logic already avoids cached `StartSpot` reuse and supports randomized starts, but round reset currently respawns controllers sequentially with no "already used this round-start pass" constraint. That means both players can still begin a fresh round on the same start. A round-start-only exclusion set solves the actual problem without broadening behavior changes for mid-round respawns.

## Implementation Scope

- Extend `APvPArenaGameMode` with a helper that chooses a start while excluding both:
  - that controller's previous start, when possible
  - starts already used in the current round-reset pass
- Update `ResetAllPlayersForNextRound()` to track starts used during that reset.
- Add focused automation coverage proving two players get distinct starts during a round reset when at least two starts exist.

## Non-Goals

- Do not add distance-based spawn weighting.
- Do not change individual death respawn timing.
- Do not require globally unique starts across an entire match.

## Verification

- Build the editor target.
- Run focused automation coverage for round-start uniqueness and existing respawn selection behavior.
- Manually verify in PIE that both players start a new round on different starts when multiple `PlayerStart` actors are placed.
