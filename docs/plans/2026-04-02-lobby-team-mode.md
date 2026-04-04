# Lobby Team Mode Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a 6-player lobby with host-selectable free-for-all or team mode, player-driven team selection, asymmetric teams, and mode-change resets for ready and team state.

**Architecture:** Extend lobby state through replicated fields on `APvPArenaPlayerState`, keep validation and reset rules centralized in `APvPArenaGameMode`, expose host mode changes and player team selection via controller RPCs, and rebuild the code HUD lobby panel around the replicated state. Keep tests focused on pure game-rule helpers and UI API surface where possible.

**Tech Stack:** Unreal Engine 5 C++, replicated `APlayerState`, server RPCs, code-built UMG widgets, automation tests

---

### Task 1: Lock down lobby team-mode rules in tests

**Files:**
- Modify: `Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp`
- Modify: `Source/PvPArena/Private/Tests/PlayerStateRoundResetTest.cpp`
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetOnlineControlsTest.cpp`

**Step 1: Write the failing tests**

Add expectations for:
- 6-player lobby cap
- free-for-all start rule: at least 2 connected players
- team-mode start rule: at least 1 player on each side
- mode-change reset behavior for ready and team selection
- controller/HUD API surface for mode changes and team selection

**Step 2: Run tests to verify they fail**

Run targeted automation tests for the modified files and confirm missing APIs / wrong rule behavior.

**Step 3: Write minimal implementation**

Only after observing failures, add the smallest new enums, methods, and properties required for the tests to compile and fail for the right reasons.

**Step 4: Run tests to verify progress**

Re-run the same targeted tests and confirm the failures move from missing APIs to rule mismatches, then to green as implementation lands.

### Task 2: Replicate mode and team state through player state

**Files:**
- Modify: `Source/PvPArena/Public/Game/PvPArenaPlayerState.h`
- Modify: `Source/PvPArena/Private/Game/PvPArenaPlayerState.cpp`

**Step 1: Write the failing test**

Extend the player-state reset test to require replicated match mode preference and team assignment accessors plus reset helpers.

**Step 2: Run test to verify it fails**

Run the player-state automation test and confirm the new fields/helpers are absent.

**Step 3: Write minimal implementation**

Add enums plus replicated fields/accessors for lobby match mode and team side, and reset helpers that preserve the confirmed behavior.

**Step 4: Run test to verify it passes**

Re-run the player-state automation test and confirm all expectations pass.

### Task 3: Implement lobby validation and mode/team reset rules

**Files:**
- Modify: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Modify: `Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`

**Step 1: Write the failing test**

Extend the match rule test to cover lobby capacity, mode-aware start validation, and mode-switch reset behavior.

**Step 2: Run test to verify it fails**

Run the match rule automation test and confirm current logic still only checks connected-player count.

**Step 3: Write minimal implementation**

Add helper functions for counting connected players by team, validating start conditions by mode, enforcing max players, and resetting ready/team state on mode changes.

**Step 4: Run test to verify it passes**

Re-run the match rule automation test and confirm the new lobby rules are green.

### Task 4: Expose host mode changes and player team selection

**Files:**
- Modify: `Source/PvPArena/Public/Game/PvPArenaPlayerController.h`
- Modify: `Source/PvPArena/Private/Game/PvPArenaPlayerController.cpp`

**Step 1: Write the failing test**

Extend the HUD/controller API test to require mode-change and team-selection request functions and backing RPCs.

**Step 2: Run test to verify it fails**

Run the API-surface automation test and confirm the functions are missing.

**Step 3: Write minimal implementation**

Add blueprint-callable controller entry points and server RPCs that forward validated requests into `APvPArenaGameMode`.

**Step 4: Run test to verify it passes**

Re-run the API-surface automation test and confirm the new controller entry points exist.

### Task 5: Rebuild lobby HUD around replicated mode/team state

**Files:**
- Modify: `Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Modify: `Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`
- Test: `Source/PvPArena/Private/Tests/HUDWidgetOnlineControlsTest.cpp`

**Step 1: Write the failing test**

Require lobby widget members/helpers for mode selection, team list display, and team selection actions.

**Step 2: Run test to verify it fails**

Run the HUD automation test and confirm the new fields/functions do not exist yet.

**Step 3: Write minimal implementation**

Add the new lobby widgets and refresh logic:
- host mode buttons
- per-team player list text
- local player team status
- left/right team selection buttons
- mode-aware lobby status copy

**Step 4: Run test to verify it passes**

Re-run the HUD automation test and confirm the widget surface is present.

### Task 6: Verify the integrated change

**Files:**
- Verify only

**Step 1: Run targeted automation coverage**

Run the updated tests for match rules, player-state resets, and HUD/controller exposure.

**Step 2: Run a broader regression slice if build cost is acceptable**

Run nearby match/HUD automation tests to catch unintended regressions.

**Step 3: Review diff**

Inspect edited files to confirm no unrelated behavior was changed.
