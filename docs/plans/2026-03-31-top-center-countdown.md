# Top Center Countdown Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Separate the countdown timer from the info panel and display it at the top center of the HUD.

**Architecture:** Add a dedicated `CountdownPanel` with its own centered overlay slot, then move `TimerText` out of `InfoBox` into that new panel. Keep the timer update logic unchanged so only widget structure and placement move.

**Tech Stack:** Unreal Engine 5 C++, UMG/Slate widget tree, Automation Tests

---

### Task 1: Update layout expectations first

**Files:**
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`

**Step 1: Write the failing test**

Add assertions that:
- a `CountdownPanel` exists
- the root overlay now includes the countdown layer
- `TimerText` belongs to the countdown panel instead of the info panel
- the countdown panel is pinned to the top center
- the info panel child count drops because the timer moved out

**Step 2: Run test to verify it fails**

Run: `Automation RunTests PvPArena.UI.HUDWidgetLayout`
Expected: FAIL because the timer is still inside the info panel.

**Step 3: Write minimal implementation**

Add the countdown panel and reparent `TimerText`.

**Step 4: Run test to verify it passes**

Run: `Automation RunTests PvPArena.UI.HUDWidgetLayout`
Expected: PASS

### Task 2: Rebuild the HUD tree with a dedicated countdown panel

**Files:**
- Modify: `Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Modify: `Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`

**Step 1: Add countdown containers**

Add:
- `CountdownPanel`
- `CountdownBox`

**Step 2: Move timer text**

Place `TimerText` inside the countdown box and anchor the countdown panel at top center.

**Step 3: Keep current timer logic**

Do not change countdown text generation or match-state timing behavior.

### Task 3: Focused verification

**Files:**
- Modify: none

**Step 1: Build**

Run the `PvPArenaEditor` build.
Expected: PASS

**Step 2: Run targeted automation**

Run: `Automation RunTests PvPArena.UI.HUDWidgetLayout+PvPArena.UI.HUDWidgetRangedCrosshair`
Expected: PASS
