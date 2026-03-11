# Final Default Restore Prep Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Make the future restoration of the original match defaults explicit and test-backed without changing the current short iteration defaults.

**Architecture:** Keep the current active defaults in `APvPArenaGameMode`, but add explicit restore-target constants/accessors for the original MVP values and assert them in the existing gameplay tempo regression test. Update the active handoff document so future sessions do not rely on stale notes.

**Tech Stack:** Unreal Engine 5.5 C++, Unreal Automation Tests, Markdown docs

---

### Task 1: Add failing coverage for restore targets

**Files:**
- Modify: `Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp`
- Modify: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Test: `Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp`

**Step 1: Write the failing test**

Extend `GameplayTempoDefaultsTest` so it asserts:
- current iteration `ScoreLimit` default is `3`
- current iteration `RoundDurationSeconds` default is `60`
- planned final restore `ScoreLimit` is `5`
- planned final restore `RoundDurationSeconds` is `180`

**Step 2: Run test to verify it fails**

Run the project build and the focused automation test.
Expected: FAIL because the restore-target helpers do not exist yet.

**Step 3: Write minimal implementation**

Add explicit restore-target constants/accessors to `APvPArenaGameMode` without changing active gameplay values.

**Step 4: Run test to verify it passes**

Run the same focused automation test again.
Expected: PASS

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Game/PvPArenaGameMode.h Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp
git commit -m "test: lock final default restore targets"
```

### Task 2: Refresh the active handoff for future restoration

**Files:**
- Modify: `docs/plans/next-session.md`

**Step 1: Write the handoff update**

Revise the handoff so it matches the current merged `main` state and clearly records:
- gameplay tempo tuning already merged
- HUD readability polish already merged
- elimination respawn boundary regression coverage already merged
- `ScoreLimit = 3` and `RoundDurationSeconds = 60` stay active for now
- final restore later must set them back to `5 / 180`

**Step 2: Review for staleness**

Confirm old branch-specific or pre-merge notes are removed.

**Step 3: Commit**

```bash
git add docs/plans/next-session.md
git commit -m "docs: refresh next-session handoff"
```

### Task 3: Verify prep work

**Files:**
- Test: `Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp`

**Step 1: Run focused automation test**

Run `PvPArena.Match.GameplayTempoDefaults`.
Expected: PASS

**Step 2: Run project build**

Run the Unreal editor build for the worktree.
Expected: build succeeds.

**Step 3: Review diff**

Inspect the branch diff to confirm only restore-prep code/test/doc changes are present.

**Step 4: Commit**

```bash
git add docs/plans/2026-03-11-final-default-restore-design.md docs/plans/2026-03-11-final-default-restore-prep.md
git commit -m "docs: capture final default restore prep"
```
