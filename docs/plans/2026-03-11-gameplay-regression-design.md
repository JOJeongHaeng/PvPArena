# Gameplay Regression Coverage Design

## Summary

This pass adds regression coverage for the death, respawn, and round-transition boundary rules without changing match behavior. The goal is to lock in the intended `APvPArenaGameMode` decision-making when an elimination happens during an active round versus after a winner has already been decided.

## Decisions

- Keep gameplay behavior unchanged.
- Extract the respawn scheduling decision from `HandlePlayerEliminated()` into a testable rule.
- Add focused automation coverage for elimination boundary cases rather than a full PIE integration test.

## Rationale

The current code behaves correctly but the key boundary rule is implicit inside `HandlePlayerEliminated()`: an eliminated player should only schedule an individual respawn when the round is still active. Once a winner has been decided, the round reset flow should take over instead.

That rule is important enough to protect, but a world-backed integration test would add more complexity than value for this stage of the project. A narrow rule-extraction approach gives stable coverage with low maintenance cost.

## Implementation Scope

- Add a pure/read-only `APvPArenaGameMode` helper that answers whether an elimination should schedule a respawn.
- Reuse that helper in `HandlePlayerEliminated()` so the tested rule matches runtime behavior.
- Add a focused automation test for active-round, no-victim, and round-already-won cases.

## Non-Goals

- Do not change timer durations or match defaults.
- Do not add full world simulation or PIE integration coverage.
- Do not alter HUD behavior or player-state scoring behavior.

## Verification

- Build the editor target.
- Run the focused gameplay automation tests covering:
  - `PvPArena.Match.WinCondition`
  - new elimination boundary regression test
  - existing death/respawn sanity coverage where relevant
