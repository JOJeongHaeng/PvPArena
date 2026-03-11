# HUD Readability Polish Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Improve the readability of the code-built HUD by strengthening hierarchy, spacing, and contrast while keeping the current runtime UMG architecture intact.

**Architecture:** Keep `UPvPArenaHUDWidget` as the existing runtime-built widget and confine this pass to styling and layout property updates. Preserve the current widget tree shape so the change remains low-risk and testable with focused UI automation coverage.

**Tech Stack:** Unreal Engine 5.5 C++, UMG, Unreal Automation Tests

---

### Task 1: Lock readability expectations with a failing layout test

**Files:**
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`
- Test: `Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`

**Step 1: Write the failing test**

Add assertions for the readability polish defaults, such as:
- health bar desired height or thickness cue
- stronger font sizes for status text rows
- announcement text fonts staying larger than status text
- non-default color/visibility expectations that represent the new hierarchy

**Step 2: Run test to verify it fails**

Run the UI automation test suite.
Expected: FAIL because the current widget styling does not yet match the new readability assertions.

**Step 3: Write minimal implementation**

Update `UPvPArenaHUDWidget` styling values just enough to satisfy the new expectations.

**Step 4: Run test to verify it passes**

Run the same UI automation test suite again.
Expected: PASS

**Step 5: Commit**

```bash
git add Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp
git commit -m "test: lock hud readability defaults"
```

### Task 2: Apply HUD readability polish

**Files:**
- Modify: `Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`
- Modify: `Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Test: `Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`

**Step 1: Write the failing test**

Extend the layout test to capture the final intended styling values and panel-level readability cues.

**Step 2: Run test to verify it fails**

Run the focused UI test command.
Expected: FAIL until the widget tree styling is updated.

**Step 3: Write minimal implementation**

- add a status background panel or equivalent styling support
- increase padding and font sizes for always-on text rows
- increase health bar visual weight
- strengthen round-end announcement styling

**Step 4: Run test to verify it passes**

Run the focused UI automation tests again.
Expected: PASS

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/UI/PvPArenaHUDWidget.h Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp
git commit -m "feat: polish hud readability"
```

### Task 3: Verify build and UI regression health

**Files:**
- Test: `Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`
- Test: `Source/PvPArena/Private/Tests/PlayerControllerHUDDefaultsTest.cpp`
- Test: `Source/PvPArena/Private/Tests/HUDWidgetHealthFallbackTest.cpp`

**Step 1: Run focused UI automation tests**

Run `PvPArena.UI`.
Expected: PASS

**Step 2: Run project build**

Run the Unreal editor build for the worktree.
Expected: build succeeds.

**Step 3: Review diff**

Inspect the branch diff to confirm only HUD readability styling, layout assertions, and docs changed.

**Step 4: Commit**

```bash
git add docs/plans/2026-03-11-hud-readability-design.md docs/plans/2026-03-11-hud-readability-polish.md
git commit -m "docs: capture hud readability plan"
```
