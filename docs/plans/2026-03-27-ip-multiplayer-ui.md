# IP Multiplayer UI Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace EOS lobby actions with listen-server host and direct IP join controls that work in packaged builds.

**Architecture:** Keep the existing HUD-driven lobby flow, but swap the EOS-specific widgets for a text input and two travel buttons. The HUD will build the travel command strings and execute them through the owning player controller.

**Tech Stack:** Unreal Engine 5 C++, UMG widget tree, automation tests

---

### Task 1: Update HUD control tests

**Files:**
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetOnlineControlsTest.cpp`
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`

**Step 1: Write the failing test**

- Replace EOS control assertions with `ConnectionStatusText`, `JoinAddressTextBox`, `HostMatchButton`, `JoinByIpButton`, and host/join handler expectations.
- Update lobby child-count and widget-presence expectations to match the new layout.

**Step 2: Run test to verify it fails**

Run: `Automation RunTests PvPArena.UI.HUDWidgetOnlineControls+PvPArena.UI.HUDWidgetLayout`

Expected: FAIL because the HUD still exposes EOS widgets and handlers.

### Task 2: Replace lobby controls in the HUD

**Files:**
- Modify: `Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Modify: `Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`

**Step 1: Write minimal implementation**

- Add IP-join widgets and handlers.
- Build `Host Match` and `Join By IP` controls into `LobbyBox`.
- Remove EOS status/button construction from the lobby panel.
- Execute internal travel commands from button handlers.

**Step 2: Run test to verify it passes**

Run: `Automation RunTests PvPArena.UI.HUDWidgetOnlineControls+PvPArena.UI.HUDWidgetLayout`

Expected: PASS

### Task 3: Run regression verification

**Files:**
- Modify: none

**Step 1: Run targeted regression checks**

Run:
- `Build.bat PvPArenaEditor Win64 Development -Project="C:\UE5CPP\PvPArena\PvPArena.uproject" -WaitMutex -NoHotReloadFromIDE`
- `Automation RunTests PvPArena.UI.HUDWidgetOnlineControls+PvPArena.UI.HUDWidgetLayout+PvPArena.UI.HUDWidgetBackgroundMusic+PvPArena.Character.AttackAudioDefaults`

Expected: PASS
