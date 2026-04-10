# Lobby Menu And Korean UI Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a top-right lobby menu button that opens the existing settings panel and translate the code-built HUD text to Korean.

**Architecture:** Extend `UPvPArenaHUDWidget` with one additional lobby-only button anchored to the root overlay, wired into the existing `ToggleSettingsMenu()` flow. Translate the HUD by replacing English literals and helper-returned status strings in the same widget class, then lock behavior with focused automation tests.

**Tech Stack:** Unreal Engine 5.5 C++, runtime-built UMG widgets, Automation Tests

---

### Task 1: Add failing HUD layout and menu tests

**Files:**
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetSettingsMenuTest.cpp`

**Step 1: Write the failing test**
- Assert a `LobbyMenuButton` and `LobbyMenuButtonText` exist.
- Assert the button is attached to the root overlay top-right.
- Assert a `HandleLobbyMenuButtonClicked` function exists.
- Assert the default label is Korean.

**Step 2: Run test to verify it fails**
- Run the focused HUD tests.

**Step 3: Write minimal implementation**
- Add the new button and handler declarations only after failure is confirmed.

**Step 4: Run test to verify it passes**
- Re-run the same focused tests.

### Task 2: Implement the lobby menu button

**Files:**
- Modify: `Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Modify: `Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`

**Step 1: Add the button widgets and click handler**
- Create `LobbyMenuButton` and `LobbyMenuButtonText`.
- Add `HandleLobbyMenuButtonClicked()`.

**Step 2: Add the button to the overlay**
- Anchor it at the top-right.
- Keep it hidden outside the lobby.

**Step 3: Reuse the settings menu flow**
- Route click handling through `ToggleSettingsMenu()`.

**Step 4: Run the focused HUD tests**
- Confirm layout/menu tests pass.

### Task 3: Add failing Korean text assertions

**Files:**
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetOnlineControlsTest.cpp`

**Step 1: Write the failing test**
- Assert key lobby/settings labels are Korean.
- Assert host/join button labels and helper-produced travel status labels still exist but are localized.

**Step 2: Run test to verify it fails**
- Run the focused HUD tests.

**Step 3: Implement minimal localization**
- Replace English text literals in `UPvPArenaHUDWidget`.

**Step 4: Run test to verify it passes**
- Re-run the focused HUD tests.

### Task 4: Translate runtime HUD helper strings

**Files:**
- Modify: `Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`

**Step 1: Translate helper-returned strings**
- Match phase labels
- Round state labels
- Lobby status text
- Team labels
- Match result and spectator help text

**Step 2: Keep dynamic values intact**
- Preserve score counts, timers, port numbers, and addresses.

**Step 3: Run focused HUD automation**
- Verify no regression in layout/settings/online tests.

### Task 5: Final verification

**Files:**
- Modify: none

**Step 1: Build**
- Run the editor build.

**Step 2: Run focused tests**
- `PvPArena.UI.HUDWidgetLayout`
- `PvPArena.UI.HUDWidgetSettingsMenu`
- `PvPArena.UI.HUDWidgetOnlineControls`

**Step 3: Report remaining manual verification**
- Confirm in PIE that the lobby top-right button opens settings and all visible HUD labels show Korean text.
