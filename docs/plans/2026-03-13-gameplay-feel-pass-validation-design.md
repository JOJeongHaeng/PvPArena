# Gameplay Feel Pass Validation Design

## Summary

This pass prioritizes manual PIE validation of the recently merged gameplay tempo and spawn-selection rules before restoring final match defaults. The goal is to confirm that the tuned respawn timings and explicit `PlayerStart` rules feel correct in live play, and to make only the smallest code or map changes needed if a real issue is observed.

## Decisions

- Start from a fresh `feature/...` branch in an isolated worktree instead of working on `main`.
- Reuse the existing test map and currently present local map asset changes during manual validation.
- Validate three areas first in PIE:
  - `RespawnDelaySeconds = 2`
  - `RespawnInvulnerabilitySeconds = 1.25f`
  - `RoundEndDelaySeconds = 3`
- Confirm that respawns vary across multiple `PlayerStart` actors and do not immediately repeat the prior start when alternatives exist.
- Confirm that round resets distribute active players across distinct starts when the map has enough `PlayerStart` actors.
- Make no final default restoration changes in this pass. `ScoreLimit = 5` and `RoundDurationSeconds = 180` remain deferred.

## Recommended Approach

Use a manual-verification-first workflow. Run the game in PIE with the current short iteration defaults and observe the merged spawn rules in the existing test map. If the feel pass exposes a concrete issue, prefer the smallest fix that addresses the observed problem:

1. adjust test-map `PlayerStart` placement if the issue is content-driven
2. make a focused gameplay-code change if the rule itself is wrong
3. add or extend automated coverage if a behavior gap was discovered during validation

This keeps the pass narrow and preserves the current MVP HUD/runtime direction.

## Scope

- Manual editor validation of tempo and spawn behavior
- Minimal gameplay code changes only if validation reveals a concrete problem
- Minimal test-map asset updates only if layout blocks meaningful validation
- Targeted automated test reruns for spawn and tempo behavior after any code change
- Short written validation notes for the next session

## Non-Goals

- Do not migrate the HUD to Widget Blueprints.
- Do not refactor unrelated gameplay systems.
- Do not restore the final `ScoreLimit` or `RoundDurationSeconds` in this pass.
- Do not add advanced spawn heuristics such as distance weighting or combat-awareness.

## Verification

- Manual PIE pass covering:
  - respawn pacing
  - post-respawn invulnerability feel
  - round-end downtime
  - respawn start variation
  - distinct round-start placement with multiple players
- Re-run relevant automated tests if code changes are made.
- Record the validation outcome in `docs/plans` so the final default restoration session starts with explicit evidence.
