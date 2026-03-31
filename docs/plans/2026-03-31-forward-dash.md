# Forward Dash Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace the hold-to-sprint movement buff with an instant dash that fires in the current facing direction.

**Architecture:** Keep the existing sprint resource bar, but reinterpret it as dash charge. `BeginSprintInput()` becomes a one-shot dash trigger that pushes velocity forward based on control yaw or actor yaw fallback, while `UpdateSprintState()` only handles brief active-state decay and recharge.

**Tech Stack:** Unreal Engine 5 C++, CharacterMovementComponent, Automation Tests

---

### Task 1: Rewrite the sprint test around dash behavior

**Files:**
- Modify: `Source/PvPArena/Private/Tests/CharacterSprintStateTest.cpp`

**Step 1: Write the failing test**

Replace the hold-sprint expectations with dash expectations:
- pressing sprint should immediately create forward velocity
- one dash should consume the full charge
- empty charge should block another dash
- recharge should refill the gauge and allow a new dash

**Step 2: Run test to verify it fails**

Run: `Automation RunTests PvPArena.Character.SprintState`
Expected: FAIL because the current implementation still uses walk-speed boost behavior.

**Step 3: Write minimal implementation**

Convert sprint input handling to dash handling and preserve only the shared resource model.

**Step 4: Run test to verify it passes**

Run: `Automation RunTests PvPArena.Character.SprintState`
Expected: PASS

### Task 2: Replace movement buff logic with instant dash logic

**Files:**
- Modify: `Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`

**Step 1: Add dash helpers and tuning**

Add helpers for:
- resolving dash direction from controller yaw or actor yaw
- activating dash
- applying dash velocity

**Step 2: Simplify sprint update behavior**

Update sprint state so it:
- no longer scales `MaxWalkSpeed`
- briefly marks the dash active for UI feedback
- recharges the dash resource over time

**Step 3: Preserve multiplayer entry points**

Reuse the existing sprint RPC so client input still asks the server to trigger the same dash.

**Step 4: Run test to verify it passes**

Run: `Automation RunTests PvPArena.Character.SprintState`
Expected: PASS

### Task 3: Align HUD wording and run focused regression checks

**Files:**
- Modify: `Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`

**Step 1: Update player-facing wording**

Change the HUD and lobby controls wording from sprint to dash where the old label would now be misleading.

**Step 2: Run focused verification**

Run:
- `Automation RunTests PvPArena.Character.SprintState`
- `Automation RunTests PvPArena.UI.HUDWidgetLayout`

Expected: PASS

### Task 4: Final verification

**Files:**
- Modify: none

**Step 1: Build the editor target**

Run the `PvPArenaEditor` build.
Expected: build succeeds.

**Step 2: Run combined targeted automation**

Run: `Automation RunTests PvPArena.Character.SprintState+PvPArena.UI.HUDWidgetLayout+PvPArena.UI.HUDWidgetRangedCrosshair`
Expected: PASS
