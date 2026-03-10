# Code-Based UMG HUD Polish Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Keep the current code-built UMG HUD path for the MVP, remove leftover debug scaffolding, and preserve the existing validated runtime behavior.

**Architecture:** Retain `APvPArenaPlayerController` as the sole HUD owner and keep `UPvPArenaHUDWidget` as a runtime-built widget. Limit this pass to low-risk cleanup and presentation-safe adjustments rather than another UI architecture change.

**Tech Stack:** Unreal Engine 5.5, C++, Unreal Automation Tests, Listen Server PIE

---

### Task 1: Remove obsolete HUD creation debug scaffolding

**Files:**
- Modify: `Source/PvPArena/Private/Game/PvPArenaPlayerController.cpp`
- Test: existing `Source/PvPArena/Private/Tests/PlayerControllerHUDDefaultsTest.cpp`

**Step 1: Verify current regression coverage**

Use `PvPArena.UI.PlayerControllerHUDDefaults` as the ownership regression check.

**Step 2: Write minimal cleanup**

- Remove temporary success-path logging from `TryCreateHUDWidget()`
- Remove includes or calls that only supported temporary debugging
- Keep retry behavior and fallback safety intact

**Step 3: Rebuild and run UI automation tests**

Run:
- `Build.bat PvPArenaEditor Win64 Development -Project="C:\UE5CPP\PvPArena\PvPArena.uproject" -WaitMutex -NoHotReloadFromIDE`
- `UnrealEditor-Cmd.exe C:\UE5CPP\PvPArena\PvPArena.uproject -ExecCmds="Automation RunTests PvPArena.UI; Quit" -unattended -nop4 -nosplash -NullRHI -TestExit="Automation Test Queue Empty" -log`

Expected:
- Build succeeds
- `PvPArena.UI.HUDWidgetLayout` passes
- `PvPArena.UI.PlayerControllerHUDDefaults` passes

### Task 2: Keep the HUD direction documented for future sessions

**Files:**
- Modify: `docs/plans/next-session.md`

**Step 1: Record the chosen direction**

- Keep the current code-based UMG HUD for the MVP
- Re-evaluate Widget Blueprint migration only during polish/finalization
- Keep `ScoreLimit = 3` and `RoundDurationSeconds = 60` until completion-stage tuning
