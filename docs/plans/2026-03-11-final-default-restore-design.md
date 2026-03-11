# Final Default Restore Prep Design

## Summary

This pass prepares the project for a later restoration of the original longer-form match defaults without changing current gameplay behavior. The goal is to keep the short iteration defaults active now, while making the future `ScoreLimit = 5` and `RoundDurationSeconds = 180` restoration path explicit in code, tests, and handoff documentation.

## Decisions

- Keep the current iteration defaults active:
  - `ScoreLimit = 3`
  - `RoundDurationSeconds = 60`
- Record the intended final restore targets in code instead of only leaving them in old docs.
- Extend existing tempo-default coverage so tests document both the current iteration defaults and the future restore targets.
- Refresh `docs/plans/next-session.md` so it reflects the actual current state of `main`.

## Rationale

The current restore plan exists mostly as tribal knowledge spread across older handoff notes and the original MVP plan. That is enough to forget or partially restore later. Putting the iteration defaults and final restore targets side by side in the active game mode and its regression tests lowers the chance of restoring only one value or restoring at the wrong phase.

This should not change behavior today. It only makes the restore point visible and verifiable.

## Implementation Scope

- Add named constants or helper accessors on `APvPArenaGameMode` for:
  - current iteration defaults
  - planned final restore defaults
- Update `GameplayTempoDefaultsTest` to assert both sets of values.
- Update `docs/plans/next-session.md` so it reflects:
  - gameplay tempo tuning is already merged
  - HUD readability polish is already merged
  - regression coverage is already merged
  - final default restoration remains a later-stage TODO

## Non-Goals

- Do not change live gameplay defaults from `3 / 60` yet.
- Do not alter respawn or round-end timing again.
- Do not add new HUD or gameplay behavior.

## Verification

- Build the editor target.
- Run `PvPArena.Match.GameplayTempoDefaults`.
- Confirm the handoff document matches the merged project state.
