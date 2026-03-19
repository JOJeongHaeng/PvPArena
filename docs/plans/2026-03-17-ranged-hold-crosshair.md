# Ranged Hold Crosshair Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Show a centered aiming reticle only while the player is actively holding right-click for the ranged charge.

**Architecture:** Reuse the existing code-built HUD widget and the character's ranged charge state. Expose a read-only hold-state accessor on `APvPArenaCharacter`, add a centered crosshair layer to `UPvPArenaHUDWidget`, and update only the reticle visibility every frame so release hides it immediately without waiting for the slower status refresh timer.

**Tech Stack:** Unreal Engine 5 C++, UMG `UUserWidget`, automation tests.

---

### Task 1: Lock the expected HUD reticle layout and visibility rules

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`
- Create: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetRangedCrosshairTest.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`

**Step 1: Write the failing test**

Add assertions that:
- the HUD root overlay now contains a centered reticle layer
- the reticle widget exists and starts hidden
- reticle visibility is `Visible` while ranged charge input is held
- reticle visibility returns to `Collapsed` immediately on release

**Step 2: Run test to verify it fails**

Run:

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.UI.HUDWidgetLayout+PvPArena.UI.HUDWidgetRangedCrosshair; Quit'"
```

Expected: FAIL because the HUD does not build a reticle and exposes no helper for its visibility rule.

**Step 3: Write minimal implementation**

Add:
- a ranged-hold accessor on `APvPArenaCharacter`
- a HUD helper that maps character state to reticle visibility
- a centered crosshair text block in the overlay

**Step 4: Run test to verify it passes**

Run the same command again.

Expected: PASS.

**Step 5: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Public/Player/PvPArenaCharacter.h Source/PvPArena/Public/UI/PvPArenaHUDWidget.h Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp Source/PvPArena/Private/Tests/HUDWidgetRangedCrosshairTest.cpp docs/plans/2026-03-17-ranged-hold-crosshair.md
git -C /mnt/c/UE5CPP/PvPArena commit -m "feat: show reticle during ranged charge hold"
```
