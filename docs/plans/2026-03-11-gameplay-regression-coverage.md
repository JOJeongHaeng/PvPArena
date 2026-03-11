# Gameplay Regression Coverage Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Lock in the intended death, respawn, and round-transition boundary rule so eliminations only schedule individual respawns while a round is still active.

**Architecture:** Keep the runtime match flow unchanged, but extract the respawn scheduling decision from `APvPArenaGameMode::HandlePlayerEliminated()` into a pure helper that tests can call directly. Cover the key elimination boundary cases with focused automation tests rather than a heavyweight world integration test.

**Tech Stack:** Unreal Engine 5.5 C++, Unreal Automation Tests

---

### Task 1: Add a failing regression test for elimination respawn boundaries

**Files:**
- Create: `Source/PvPArena/Private/Tests/EliminationRespawnBoundaryTest.cpp`
- Modify: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Test: `Source/PvPArena/Private/Tests/EliminationRespawnBoundaryTest.cpp`

**Step 1: Write the failing test**

Add an automation test that creates `APvPArenaGameMode` and asserts:
- active round + victim controller present => respawn should be scheduled
- no victim controller => respawn should not be scheduled
- winner already decided => respawn should not be scheduled

**Step 2: Run test to verify it fails**

Run the project build and focused automation test.
Expected: FAIL because the helper does not exist yet.

**Step 3: Write minimal implementation**

Expose a pure helper on `APvPArenaGameMode` that returns whether an elimination should schedule a respawn.

**Step 4: Run test to verify it passes**

Run the focused automation test again.
Expected: PASS

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Game/PvPArenaGameMode.h Source/PvPArena/Private/Tests/EliminationRespawnBoundaryTest.cpp
git commit -m "test: lock elimination respawn boundaries"
```

### Task 2: Route runtime elimination logic through the tested rule

**Files:**
- Modify: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Modify: `Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`
- Test: `Source/PvPArena/Private/Tests/EliminationRespawnBoundaryTest.cpp`

**Step 1: Write the failing test**

Keep the new regression test as the required proof that the helper semantics match runtime intent.

**Step 2: Run test to verify it fails or stays red if helper semantics are incomplete**

Run the focused build/test step.
Expected: FAIL until the runtime guard uses the new helper.

**Step 3: Write minimal implementation**

- implement the helper
- replace the inline `!VictimController || bHasWinner` guard in `HandlePlayerEliminated()` with the helper-backed rule

**Step 4: Run test to verify it passes**

Run the focused automation tests again.
Expected: PASS

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Game/PvPArenaGameMode.h Source/PvPArena/Private/Game/PvPArenaGameMode.cpp Source/PvPArena/Private/Tests/EliminationRespawnBoundaryTest.cpp
git commit -m "feat: cover elimination respawn boundaries"
```

### Task 3: Verify gameplay regression health

**Files:**
- Test: `Source/PvPArena/Private/Tests/EliminationRespawnBoundaryTest.cpp`
- Test: `Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp`
- Test: `Source/PvPArena/Private/Tests/RespawnInvulnerabilityTest.cpp`

**Step 1: Run focused automation tests**

Run the gameplay regression test subset.
Expected: PASS

**Step 2: Run project build**

Run the Unreal editor build for the worktree.
Expected: build succeeds.

**Step 3: Review diff**

Inspect the branch diff to confirm only gameplay regression coverage and docs changed.

**Step 4: Commit**

```bash
git add docs/plans/2026-03-11-gameplay-regression-design.md docs/plans/2026-03-11-gameplay-regression-coverage.md
git commit -m "docs: capture gameplay regression coverage plan"
```
