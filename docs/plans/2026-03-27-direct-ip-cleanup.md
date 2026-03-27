# Direct IP Cleanup Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Remove unused EOS integration so the project matches the direct IP multiplayer workflow.

**Architecture:** Keep the HUD-based host/join flow and delete the now-unused online subsystem layer. Revert project/plugin/config changes that were only needed for EOS, then verify the remaining HUD and audio regressions still pass.

**Tech Stack:** Unreal Engine 5 C++, `.uproject` plugin config, module rules, automation tests

---

### Task 1: Write the failing cleanup test

**Files:**
- Modify: `Source/PvPArena/Private/Tests/OnlineSubsystemDefaultsTest.cpp`

**Step 1: Write the failing test**

- Replace the EOS defaults test with a direct-IP cleanup test that expects `UPvPArenaOnlineSubsystem` to no longer exist.

**Step 2: Run test to verify it fails**

Run: `Automation RunTests PvPArena.Network.DirectIpDefaults`

Expected: FAIL because `UPvPArenaOnlineSubsystem` still exists in the module.

### Task 2: Remove EOS code and config

**Files:**
- Delete: `Source/PvPArena/Public/Game/PvPArenaOnlineSubsystem.h`
- Delete: `Source/PvPArena/Private/Game/PvPArenaOnlineSubsystem.cpp`
- Modify: `.gitignore`
- Modify: `Config/DefaultEngine.ini`
- Modify: `PvPArena.uproject`
- Modify: `Source/PvPArena/PvPArena.Build.cs`
- Modify: `Source/PvPArena/Private/Tests/OnlineSubsystemDefaultsTest.cpp`

**Step 1: Write minimal implementation**

- Delete the online subsystem class.
- Remove EOS plugin/module/config additions.
- Retitle the test to direct-IP defaults and make it pass against the cleaned project.

**Step 2: Run test to verify it passes**

Run: `Automation RunTests PvPArena.Network.DirectIpDefaults`

Expected: PASS

### Task 3: Run regression verification

**Files:**
- Modify: none

**Step 1: Run targeted regression checks**

Run:
- `Build.bat PvPArenaEditor Win64 Development -Project="C:\UE5CPP\PvPArena\PvPArena.uproject" -WaitMutex -NoHotReloadFromIDE`
- `Automation RunTests PvPArena.Network.DirectIpDefaults+PvPArena.UI.HUDWidgetOnlineControls+PvPArena.UI.HUDWidgetLayout+PvPArena.UI.HUDWidgetBackgroundMusic+PvPArena.Character.AttackAudioDefaults`

Expected: PASS
