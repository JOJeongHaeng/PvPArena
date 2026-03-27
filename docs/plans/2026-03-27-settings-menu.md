# Settings Menu Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a reusable in-game settings menu with pause-style controls for audio, display, and quit actions.

**Architecture:** Extend the existing HUD widget with a settings overlay and drive it from the local player controller. Use `UGameUserSettings` for display options and keep audio controls within the HUD's existing background music/effects usage surface.

**Tech Stack:** Unreal Engine 5 C++, UMG widget tree, `UGameUserSettings`, automation tests

---

### Task 1: Add failing settings-menu tests

**Files:**
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`
- Create: `Source/PvPArena/Private/Tests/HUDWidgetSettingsMenuTest.cpp`

**Step 1: Write the failing test**

- Assert that the HUD exposes a settings panel, resume/quit buttons, and handlers for opening the menu and applying settings.

**Step 2: Run test to verify it fails**

Run: `Automation RunTests PvPArena.UI.HUDWidgetLayout+PvPArena.UI.HUDWidgetSettingsMenu`

Expected: FAIL because the settings overlay does not exist yet.

### Task 2: Implement HUD settings menu

**Files:**
- Modify: `Source/PvPArena/Public/Game/PvPArenaPlayerController.h`
- Modify: `Source/PvPArena/Private/Game/PvPArenaPlayerController.cpp`
- Modify: `Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Modify: `Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`

**Step 1: Write minimal implementation**

- Add an `Esc` binding path through the player controller into the HUD widget.
- Add the settings overlay, controls, and local apply/quit handlers.

**Step 2: Run test to verify it passes**

Run: `Automation RunTests PvPArena.UI.HUDWidgetLayout+PvPArena.UI.HUDWidgetSettingsMenu`

Expected: PASS

### Task 3: Run regression verification

**Files:**
- Modify: none

**Step 1: Run targeted regression checks**

Run:
- `Build.bat PvPArenaEditor Win64 Development -Project="C:\UE5CPP\PvPArena\PvPArena.uproject" -WaitMutex -NoHotReloadFromIDE`
- `Automation RunTests PvPArena.UI.HUDWidgetLayout+PvPArena.UI.HUDWidgetSettingsMenu+PvPArena.UI.HUDWidgetOnlineControls+PvPArena.UI.HUDWidgetBackgroundMusic+PvPArena.Character.AttackAudioDefaults+PvPArena.Network.DirectIpDefaults`

Expected: PASS
