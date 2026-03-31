# Preturn Dash Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Make idle dash feel natural by quickly rotating the character toward camera yaw before the dash fires, while also increasing dash distance.

**Architecture:** Extend the existing dash system with a short pending-turn state. On dash input, if the facing angle differs from the target camera yaw, store a pending dash target yaw, rotate rapidly in `Tick`, and apply dash velocity only once alignment is close enough; otherwise dash immediately. Increase the dash speed value so the resulting dash travels noticeably farther and naturally drives the locomotion animation.

**Tech Stack:** Unreal Engine 5 C++, Character tick/update loop, Automation Tests

---

### Task 1: Add a failing test for preturn behavior

**Files:**
- Modify: `Source/PvPArena/Private/Tests/CharacterSprintStateTest.cpp`

**Step 1: Write the failing test**

Add a case where:
- actor yaw and controller yaw are different
- dash input should not immediately create velocity
- update ticks should rotate the actor toward controller yaw
- once aligned, dash velocity should fire in the controller-facing direction

**Step 2: Run test to verify it fails**

Run: `Automation RunTests PvPArena.Character.SprintState`
Expected: FAIL because dash currently fires immediately without a preturn state.

**Step 3: Write minimal implementation**

Add the pending turn state and make the dash trigger after alignment.

**Step 4: Run test to verify it passes**

Run: `Automation RunTests PvPArena.Character.SprintState`
Expected: PASS

### Task 2: Implement preturn dash and longer range

**Files:**
- Modify: `Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`

**Step 1: Add dash-turn state**

Add fields for:
- pending dash flag
- pending dash target yaw
- turn interpolation speed
- yaw tolerance

**Step 2: Update dash activation**

When dash input happens:
- compute target yaw
- if already aligned, dash immediately
- otherwise cache the target yaw and enter pending-turn mode

**Step 3: Update per-frame behavior**

In tick/update:
- rotate rapidly toward the pending dash target yaw
- fire dash when within tolerance
- allow recharge only when no dash is active or pending

**Step 4: Increase dash distance**

Raise the dash distance tuning so the dash travels much farther.

### Task 3: Focused verification

**Files:**
- Modify: none

**Step 1: Build**

Run the `PvPArenaEditor` build.
Expected: PASS

**Step 2: Run targeted automation**

Run: `Automation RunTests PvPArena.Character.SprintState+PvPArena.UI.HUDWidgetLayout+PvPArena.UI.HUDWidgetRangedCrosshair`
Expected: PASS
