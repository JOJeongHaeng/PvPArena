# Projectile Debug Speed Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Slow the ranged projectile slightly and add draw-debug trajectory visualization so flight alignment can be inspected in-game.

**Architecture:** Keep spawn and damage behavior in the existing combat/projectile split. Put the lower default speed and the per-projectile debug trail settings on `APvPProjectile`, then let the projectile draw its own recent movement segments during flight while the combat component keeps its existing spawn/aim debug markers.

**Tech Stack:** Unreal Engine 5 C++, `UProjectileMovementComponent`, `DrawDebugHelpers`, automation tests.

---

### Task 1: Lock projectile defaults for speed and debug trail support

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RangedProjectileDefaultsTest.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Combat/PvPProjectile.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Combat/PvPProjectile.cpp`

**Step 1: Write the failing test**

Add assertions that:
- projectile initial speed is reduced from the current default
- projectile exposes a debug-enable property
- projectile exposes a debug draw duration property
- projectile exposes a debug trail thickness property

**Step 2: Run test to verify it fails**

Run:

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Combat.RangedProjectileDefaults; Quit'"
```

Expected: FAIL because the old projectile speed is still high and no projectile-local debug settings exist.

**Step 3: Write minimal implementation**

Implement:
- a slightly lower projectile speed default
- projectile tick for debug trail drawing
- edit-defaults debug properties for enable, duration, and line thickness

**Step 4: Run test to verify it passes**

Run the same command again.

Expected: PASS.

**Step 5: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Public/Combat/PvPProjectile.h Source/PvPArena/Private/Combat/PvPProjectile.cpp Source/PvPArena/Private/Tests/RangedProjectileDefaultsTest.cpp docs/plans/2026-03-17-projectile-debug-speed.md
git -C /mnt/c/UE5CPP/PvPArena commit -m "feat: add projectile debug trail tuning"
```
