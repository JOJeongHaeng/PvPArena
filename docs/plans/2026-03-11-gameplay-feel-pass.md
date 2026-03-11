# Random Respawn Starts Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Make respawn selection explicitly random across multiple `PlayerStart` actors while avoiding immediate spawn-point repeats when alternatives exist.

**Architecture:** Keep the existing `RestartPlayer()` flow, but override `APvPArenaGameMode::ChoosePlayerStart` and route the actual selection through a small helper that can be tested without a full world simulation. Persist only the last chosen start so the rule stays simple and deterministic to reason about.

**Tech Stack:** Unreal Engine 5.5 C++, Unreal Automation Tests

---

### Task 1: Add failing coverage for non-repeating spawn selection

**Files:**
- Create: `Source/PvPArena/Private/Tests/RandomRespawnStartSelectionTest.cpp`
- Modify: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Test: `Source/PvPArena/Private/Tests/RandomRespawnStartSelectionTest.cpp`

**Step 1: Write the failing test**

Add tests that assert:
- with two or more candidates, the previous spawn is not reused immediately
- with one candidate, reuse is allowed

**Step 2: Run test to verify it fails**

Run the project build and focused automation test.
Expected: FAIL because the helper does not exist yet.

**Step 3: Write minimal implementation**

Expose a small helper on `APvPArenaGameMode` that selects from a candidate array while avoiding the previous start when possible.

**Step 4: Run test to verify it passes**

Run the focused automation test again.
Expected: PASS

### Task 2: Route runtime spawn selection through the helper

**Files:**
- Modify: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Modify: `Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`
- Test: `Source/PvPArena/Private/Tests/RandomRespawnStartSelectionTest.cpp`

**Step 1: Keep the helper test as the guardrail**

Use the new test as the regression proof for the selection rule.

**Step 2: Write minimal runtime implementation**

- override `ChoosePlayerStart`
- gather `PlayerStart` actors
- pick through the helper
- remember the chosen start for next time
- fall back to `Super::ChoosePlayerStart` if no candidate exists

**Step 3: Run test to verify it still passes**

Run the focused automation test again.

### Task 3: Verify build and handoff

**Files:**
- Modify: `docs/plans/next-session.md`

**Step 1: Build the project**

Run the editor build for the worktree.

**Step 2: Run focused automation tests**

Run the new random respawn start test.

**Step 3: Update handoff**

Record that multiple `PlayerStart` actors are now intentionally randomized with anti-repeat behavior.
