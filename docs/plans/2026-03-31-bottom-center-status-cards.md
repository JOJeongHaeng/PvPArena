# Bottom Center Status Cards Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Move the HP, sprint, and ranged cooldown HUD into three horizontal cards anchored at the bottom center of the screen.

**Architecture:** Keep the existing HUD data refresh path, but rebuild the `StatusPanel` widget tree into a bottom-centered horizontal card row. Reuse the current bars and labels, only changing the layout containers and overlay slot placement.

**Tech Stack:** Unreal Engine 5 C++, UMG/Slate widget tree construction, Automation Tests

---

### Task 1: Lock the new HUD layout in tests

**Files:**
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`

**Step 1: Write the failing test**

Add assertions that:
- `StatusPanel` is anchored to `HAlign_Center` and `VAlign_Bottom`
- the status panel contains a horizontal status card row
- the row contains exactly three cards

**Step 2: Run test to verify it fails**

Run: `Automation RunTests PvPArena.UI.HUDWidgetLayout`
Expected: FAIL because the old layout is still top-right and vertically stacked.

**Step 3: Write minimal implementation**

Update the HUD widget tree to build the new status card row and bottom-center anchoring.

**Step 4: Run test to verify it passes**

Run: `Automation RunTests PvPArena.UI.HUDWidgetLayout`
Expected: PASS

### Task 2: Rebuild the status section as three cards

**Files:**
- Modify: `Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Modify: `Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`

**Step 1: Implement the new containers**

Add:
- `StatusCardsBox`
- `HealthCard`
- `SprintCard`
- `RangedCard`
- matching per-card vertical boxes

**Step 2: Rewire existing bars and labels**

Move:
- health bar + health text into the health card
- sprint bar + sprint text into the sprint card
- ranged cooldown bar + ranged cooldown text into the ranged card

**Step 3: Keep visual consistency**

Preserve current fonts, colors, and bar sizing while applying small card padding and spacing for the horizontal layout.

**Step 4: Run test to verify it passes**

Run: `Automation RunTests PvPArena.UI.HUDWidgetLayout`
Expected: PASS

### Task 3: Run focused regression verification

**Files:**
- Modify: none

**Step 1: Run the targeted UI suite**

Run: `Automation RunTests PvPArena.UI.HUDWidgetLayout+PvPArena.UI.HUDWidgetRangedCrosshair`
Expected: PASS

**Step 2: Confirm build health**

Run the editor build for `PvPArenaEditor`.
Expected: build succeeds with no new compile or link errors.
