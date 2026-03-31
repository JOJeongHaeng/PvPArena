# Default Crosshair Offset Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Move the default on-screen crosshair slightly upward during active play while keeping right-click aim state centered and leaving shot resolution unchanged.

**Architecture:** Keep the change isolated to `UPvPArenaHUDWidget`. Add a small state helper that returns the visual-only vertical offset for the crosshair, then apply that offset through widget render translation so gameplay traces continue using the existing center-based aim logic.

**Tech Stack:** Unreal Engine 5 C++, Slate/UMG widget tree, Automation Tests

---

### Task 1: Define the HUD state contract

**Files:**
- Modify: `Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Test: `Source/PvPArena/Private/Tests/HUDWidgetRangedCrosshairTest.cpp`

**Step 1: Write the failing test**

Add assertions for a new helper that:
- returns a negative vertical offset during `Playing`
- returns `0.0f` while ranged charge input is held
- returns `0.0f` when the crosshair should be hidden

**Step 2: Run test to verify it fails**

Run: `Automation RunTests PvPArena.UI.HUDWidgetRangedCrosshair`
Expected: FAIL because the helper does not exist yet.

**Step 3: Write minimal implementation**

Declare the helper in `UPvPArenaHUDWidget` and make its behavior depend only on match visibility state plus character ranged-charge hold state.

**Step 4: Run test to verify it passes**

Run: `Automation RunTests PvPArena.UI.HUDWidgetRangedCrosshair`
Expected: PASS

### Task 2: Apply the visual-only offset in the HUD

**Files:**
- Modify: `Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`
- Test: `Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`

**Step 1: Write the failing test**

Add a layout/state assertion that the crosshair widget still stays center-anchored structurally while its visual offset is handled separately.

**Step 2: Run test to verify it fails**

Run: `Automation RunTests PvPArena.UI.HUDWidgetLayout`
Expected: FAIL until the new presentation logic is wired in.

**Step 3: Write minimal implementation**

Update the HUD refresh path to:
- keep visibility behavior unchanged
- apply a negative render translation for default play
- reset render translation to zero while right-click aiming

**Step 4: Run test to verify it passes**

Run: `Automation RunTests PvPArena.UI.HUDWidgetLayout`
Expected: PASS

### Task 3: Verify no gameplay aim regression

**Files:**
- Modify: none
- Test: `Source/PvPArena/Private/Tests/HUDWidgetRangedCrosshairTest.cpp`

**Step 1: Run focused tests**

Run:
- `Automation RunTests PvPArena.UI.HUDWidgetRangedCrosshair`
- `Automation RunTests PvPArena.UI.HUDWidgetLayout`

Expected: PASS

**Step 2: Run broader safety coverage**

Run: `Automation RunTests PvPArena.UI`
Expected: PASS with no crosshair visibility regressions.
