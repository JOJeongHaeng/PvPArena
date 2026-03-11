# Gameplay Tempo Tuning Design

## Summary

This pass keeps the short iteration-friendly match defaults in place while reducing dead time between engagements. The goal is to preserve the current 60 second round and score limit of 3, but make deaths and round transitions feel less sluggish during repeated playtests.

## Decisions

- Keep `ScoreLimit = 3` and `RoundDurationSeconds = 60` unchanged for this branch.
- Reduce `RespawnDelaySeconds` from `3` to `2`.
- Reduce respawn invulnerability from `1.5s` to `1.25s`.
- Reduce `RoundEndDelaySeconds` from `5` to `3`.

## Rationale

The current short-match settings already support fast testing, but the downtime after eliminations and round wins is still long relative to the full round length. Shortening only those transition windows improves pace without changing score pressure, timeout behavior, HUD structure, or win-condition rules.

Lowering respawn invulnerability only slightly avoids creating a long protected window after a faster respawn. The values remain conservative enough to reduce immediate spawn punishes while allowing fights to restart sooner.

## Implementation Scope

- Centralize the respawn invulnerability duration in `APvPArenaGameMode` instead of duplicating `1.5f` in multiple call sites.
- Update the default tempo values in `APvPArenaGameMode`.
- Add a focused automation test that locks the default tuning values in place.

## Non-Goals

- Do not restore final MVP defaults of `ScoreLimit = 5` and `RoundDurationSeconds = 180` in this branch.
- Do not change HUD presentation or round-state behavior.
- Do not redesign spawn logic or sudden death rules.

## Verification

- Run the gameplay tempo default test and existing round/respawn automation tests.
- Build the editor target to confirm the tuning change compiles cleanly.
