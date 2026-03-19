# Ranged Release Crosshair Aim Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Remove the projectile Niagara effect and make charged ranged release follow the world point under the HUD crosshair, with smooth character turning during hold.

**Architecture:** Keep the existing hold/release ranged state machine. Move aim resolution to a character-side camera trace that updates during local hold, then let the combat component use the final crosshair-derived world point or yaw at release. Remove projectile-local Niagara so the projectile remains collision and movement only.

**Tech Stack:** Unreal Engine 5 C++, Character tick, line trace, projectile spawn, automation tests.

---

### Task 1: Lock the new projectile defaults and hold-aim expectations

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RangedProjectileDefaultsTest.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RangedAttackTimingTest.cpp`

**Step 1: Write the failing tests**

- Assert the projectile no longer has a Niagara component.
- Assert hold-time facing can update from current aim input instead of staying on the original hold-start yaw.

**Step 2: Run the tests to verify failure**

Run:

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Combat.RangedProjectileDefaults+PvPArena.Combat.RangedAttackTiming; Quit'"
```

Expected: FAIL because the projectile still expects Niagara wiring or the hold aim is not yet refreshed from live crosshair direction.

### Task 2: Remove projectile-local Niagara effect

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Combat/PvPProjectile.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Combat/PvPProjectile.cpp`

**Step 1: Write minimal implementation**

- Remove the Niagara component member and asset loading.
- Keep collision, movement, damage, lifespan, and debug trail support.

**Step 2: Run projectile defaults test**

Run:

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Combat.RangedProjectileDefaults; Quit'"
```

Expected: PASS for the no-Niagara expectation.

### Task 3: Update ranged hold to follow crosshair world aim

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Combat/PvPCombatComponent.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Combat/PvPCombatComponent.cpp`

**Step 1: Write minimal implementation**

- Add a helper that traces from the local camera through the crosshair into the world.
- While ranged hold input is active, refresh the target yaw from the latest aim point each tick.
- On release, reuse the latest crosshair-derived aim point to determine projectile direction.
- Keep server-authoritative spawning and use sane fallback when the trace misses.

**Step 2: Run ranged timing test**

Run:

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Combat.RangedAttackTiming; Quit'"
```

Expected: PASS with the refreshed hold aim behavior.

### Task 4: Build and verify the integrated result

**Files:**
- No new files expected

**Step 1: Build**

Run:

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat' PvPArenaEditor Win64 Development '-Project=C:\UE5CPP\PvPArena\PvPArena.uproject' -WaitMutex -NoHotReloadFromIDE"
```

Expected: build succeeds with exit code `0`.

**Step 2: Run targeted automation**

Run:

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Combat.RangedProjectileDefaults+PvPArena.Combat.RangedAttackTiming; Quit'"
```

Expected: PASS.

**Step 3: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Public/Combat/PvPProjectile.h Source/PvPArena/Private/Combat/PvPProjectile.cpp Source/PvPArena/Public/Player/PvPArenaCharacter.h Source/PvPArena/Private/Player/PvPArenaCharacter.cpp Source/PvPArena/Public/Combat/PvPCombatComponent.h Source/PvPArena/Private/Combat/PvPCombatComponent.cpp Source/PvPArena/Private/Tests/RangedProjectileDefaultsTest.cpp Source/PvPArena/Private/Tests/RangedAttackTimingTest.cpp docs/plans/2026-03-19-ranged-release-crosshair-aim-design.md docs/plans/2026-03-19-ranged-release-crosshair-aim.md
git -C /mnt/c/UE5CPP/PvPArena commit -m "fix: aim ranged release from crosshair target"
```
