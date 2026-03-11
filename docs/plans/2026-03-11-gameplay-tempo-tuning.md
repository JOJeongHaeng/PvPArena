# Gameplay Tempo Tuning Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Tighten the PvP loop by shortening elimination and round transition downtime while keeping the short test-match defaults unchanged.

**Architecture:** Adjust the authoritative match tempo defaults in `APvPArenaGameMode`, remove duplicated respawn invulnerability literals, and lock the intended values with focused automation coverage. Keep the existing round flow and HUD behavior intact.

**Tech Stack:** Unreal Engine 5 C++, Automation Tests

---

### Task 1: Add failing coverage for tempo defaults

**Files:**
- Create: `Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp`
- Modify: `Source/PvPArena/PvPArena.Build.cs`
- Test: `Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp`

**Step 1: Write the failing test**

Add an automation test that creates `APvPArenaGameMode` and verifies:
- `ShouldEndRoundOnKill(EPvPARoundState::Playing, 2)` is `true`
- `ShouldEndRoundOnKill(EPvPARoundState::Playing, 1)` is `false`
- default respawn delay is `2`
- default round-end delay is `3`
- default respawn invulnerability is `1.25f`

**Step 2: Run test to verify it fails**

Run: build and execute the focused automation test
Expected: FAIL because the defaults still expose `3`, `5`, and `1.5f`, or because accessors do not exist yet.

**Step 3: Write minimal implementation**

Expose read-only tempo accessors on `APvPArenaGameMode` and keep behavior unchanged except for the new desired defaults.

**Step 4: Run test to verify it passes**

Run the focused automation test again.
Expected: PASS

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Game/PvPArenaGameMode.h Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp
git commit -m "test: lock gameplay tempo defaults"
```

### Task 2: Update gameplay tempo defaults and remove duplicated literals

**Files:**
- Modify: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Modify: `Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`
- Test: `Source/PvPArena/Private/Tests/RespawnInvulnerabilityTest.cpp`
- Test: `Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp`
- Test: `Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp`

**Step 1: Write the failing test**

Extend the new tempo-default test or existing tests so they assert the updated values and accessors used by the respawn logic.

**Step 2: Run test to verify it fails**

Run the focused automation tests.
Expected: FAIL against the old defaults.

**Step 3: Write minimal implementation**

- Set `RespawnDelaySeconds = 2`
- Set `RoundEndDelaySeconds = 3`
- Add a single `RespawnInvulnerabilitySeconds = 1.25f`
- Replace both hard-coded invulnerability literals in `ResetAllPlayersForNextRound()` and `HandlePlayerEliminated()`

**Step 4: Run test to verify it passes**

Run the focused automation tests again.
Expected: PASS

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Game/PvPArenaGameMode.h Source/PvPArena/Private/Game/PvPArenaGameMode.cpp Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp
git commit -m "feat: tune gameplay tempo defaults"
```

### Task 3: Verify regression coverage and build health

**Files:**
- Test: `Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp`
- Test: `Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp`
- Test: `Source/PvPArena/Private/Tests/RespawnInvulnerabilityTest.cpp`

**Step 1: Run focused automation tests**

Run the gameplay tempo, round win condition, and respawn invulnerability tests.
Expected: PASS

**Step 2: Run project build**

Run the Unreal editor build for the project worktree.
Expected: build succeeds with no compile errors.

**Step 3: Review diff**

Inspect the branch diff to confirm only the intended tempo tuning and tests changed.

**Step 4: Commit**

```bash
git add docs/plans/2026-03-11-gameplay-tempo-design.md docs/plans/2026-03-11-gameplay-tempo-tuning.md
git commit -m "docs: capture gameplay tempo tuning plan"
```
