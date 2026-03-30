# Host Start Match Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Keep the lobby idle until the listen-server host explicitly starts the match after at least two players are connected.

**Architecture:** Reuse the existing lobby and match-phase flow, but remove all automatic start paths. The HUD should expose a host-only `Start Match` action, the player controller should forward that action to the server, and the game mode should be the single authority that validates and starts the match.

**Tech Stack:** Unreal Engine 5 C++, replicated `GameMode` / `PlayerController` / `GameState`, code-built UMG HUD, Automation tests

---

### Task 1: Lock match start behind host authority

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Game/PvPArenaPlayerController.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Game/PvPArenaPlayerController.cpp`
- Test: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp`

**Step 1: Write the failing test**

- Extend `RoundWinConditionTest.cpp` with host-only start conditions and minimum-player checks.

**Step 2: Run test to verify it fails**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.Automation.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Match.WinCondition; Quit'"`

Expected: FAIL until the host-start path is fully enforced.

**Step 3: Write minimal implementation**

- Keep lobby state on login / ready-state changes.
- Add a server RPC from `PlayerController` to `GameMode`.
- Let `GameMode` validate authority and connected-player count before calling `StartMatchFlow()`.

**Step 4: Run test to verify it passes**

Run the same automation command.

Expected: PASS.

### Task 2: Finish host-only lobby HUD behavior

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`
- Test: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetOnlineControlsTest.cpp`

**Step 1: Write the failing test**

- Add assertions for host-only start-button visibility or status messaging in the lobby HUD.

**Step 2: Run test to verify it fails**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.Automation.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.UI.HUDWidgetOnlineControls; Quit'"`

Expected: FAIL until the lobby HUD matches the host-start design.

**Step 3: Write minimal implementation**

- Show the lobby button only for the host when enough players are present.
- Change the button action to `RequestLobbyMatchStart()`.
- Update lobby status text for host / non-host / insufficient-player states.

**Step 4: Run test to verify it passes**

Run the same automation command.

Expected: PASS.

### Task 3: Verify the integrated flow

**Files:**
- No code changes unless verification exposes a defect.

**Step 1: Run targeted automation**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.Automation.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Match.WinCondition+PvPArena.UI.HUDWidgetOnlineControls; Quit'"`

Expected: PASS.

**Step 2: Run build**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat' PvPArenaEditor Win64 Development '-Project=C:\UE5CPP\PvPArena\PvPArena.uproject' -WaitMutex -NoHotReloadFromIDE'"`

Expected: build succeeds.
