# Ranged Charge Hold Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Convert right-click ranged attack into a 1.0 second hold-to-charge attack with `Start/Hold/Release/Cancel` montage sections, release-only projectile firing, cancel-without-cooldown, and a smooth temporary aim camera offset.

**Architecture:** Keep the existing montage-plus-notify combat flow, but replace one-shot ranged attack with an explicit character-side charge state machine. Input press/release decides commit vs cancel, combat cooldown is consumed only on successful release, and camera behavior is an additive offset over the existing camera rather than a new camera rig.

**Tech Stack:** Unreal Engine 5 C++, AnimMontage sections, Enhanced Input, replicated actor/component state, automation tests.

---

### Task 1: Lock in failing ranged charge timing tests

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RangedAttackTimingTest.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`

**Step 1: Write the failing test**

Add assertions for:

- press starts ranged charge but does not consume cooldown yet
- releasing before 1.0 seconds cancels without triggering hit
- releasing after 1.0 seconds allows release path and hit notify
- finish restores movement and facing lock

**Step 2: Run test to verify it fails**

Run:

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Combat.RangedAttackTiming; Quit'"
```

Expected: FAIL because the old ranged flow has no hold/release/cancel state.

**Step 3: Write minimal implementation**

Add placeholder APIs and state needed by the test:

- `BeginRangedCharge`
- `ReleaseRangedCharge`
- ranged charge timestamps / flags
- ability to distinguish cancel vs committed release

**Step 4: Run test to verify partial progress**

Run the same test again.

Expected: fewer failures, but montage section behavior still missing.

**Step 5: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Private/Tests/RangedAttackTimingTest.cpp Source/PvPArena/Public/Player/PvPArenaCharacter.h Source/PvPArena/Private/Player/PvPArenaCharacter.cpp
git -C /mnt/c/UE5CPP/PvPArena commit -m "test: define ranged charge hold timing expectations"
```

### Task 2: Convert input from one-shot ranged attack to press/release charge flow

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`
- Modify: any player input binding file discovered for right-click action

**Step 1: Write the failing test**

Extend or add tests asserting:

- right-click press enters charging state
- right-click release before threshold cancels
- right-click release after threshold commits release

**Step 2: Run test to verify it fails**

Run:

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Combat.RangedAttackTiming; Quit'"
```

Expected: FAIL until input and release handling are wired.

**Step 3: Write minimal implementation**

Implement:

- press handler for ranged charge start
- release handler for ranged charge resolution
- hold threshold constant of `1.0f`
- movement suppression and facing lock during charge

**Step 4: Run test to verify it passes**

Run the same test again.

Expected: PASS for input/state transitions.

**Step 5: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Public/Player/PvPArenaCharacter.h Source/PvPArena/Private/Player/PvPArenaCharacter.cpp
git -C /mnt/c/UE5CPP/PvPArena commit -m "feat: add press release ranged charge state flow"
```

### Task 3: Drive montage sections for start, hold, release, and cancel

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`

**Step 1: Write the failing test**

Add assertions for:

- charge starts at `Start`
- hold loops until release/cancel is requested
- short release routes to cancel
- valid release routes to release section

**Step 2: Run test to verify it fails**

Run:

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Combat.RangedAttackTiming; Quit'"
```

Expected: FAIL because section routing is not implemented yet.

**Step 3: Write minimal implementation**

Implement montage helpers:

- jump to `Start` on charge begin
- set `Hold` loop behavior
- jump to `Release` on committed release
- jump to `Cancel` on early release
- keep montage end delegate cleanup

**Step 4: Run test to verify it passes**

Run the same test again.

Expected: PASS for section routing expectations.

**Step 5: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Public/Player/PvPArenaCharacter.h Source/PvPArena/Private/Player/PvPArenaCharacter.cpp
git -C /mnt/c/UE5CPP/PvPArena commit -m "feat: route ranged charge montage sections"
```

### Task 4: Move ranged cooldown consumption to successful release

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Combat/PvPCombatComponent.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Combat/PvPCombatComponent.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/CombatCooldownValidationTest.cpp`

**Step 1: Write the failing test**

Add assertions for:

- starting charge does not update ranged cooldown
- cancel does not update ranged cooldown
- committed release updates ranged cooldown
- replicated cooldown property still exists

**Step 2: Run test to verify it fails**

Run:

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Combat.CooldownValidation; Quit'"
```

Expected: FAIL because cooldown is currently consumed at attack start.

**Step 3: Write minimal implementation**

Move cooldown consumption to the release-commit path and keep replication unchanged.

**Step 4: Run test to verify it passes**

Run the same test again.

Expected: PASS.

**Step 5: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Public/Combat/PvPCombatComponent.h Source/PvPArena/Private/Combat/PvPCombatComponent.cpp Source/PvPArena/Private/Tests/CombatCooldownValidationTest.cpp
git -C /mnt/c/UE5CPP/PvPArena commit -m "fix: consume ranged cooldown only on committed release"
```

### Task 5: Keep projectile firing notify release-only and cancel-safe

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RangedAttackTimingTest.cpp`

**Step 1: Write the failing test**

Add assertions for:

- release notify fires exactly once after committed release
- cancel path never fires projectile

**Step 2: Run test to verify it fails**

Run the ranged timing test again.

Expected: FAIL until notify gating understands cancel vs release.

**Step 3: Write minimal implementation**

Gate `HandleRangedAttackHitNotify` and `TriggerRangedAttackHit` behind a committed release flag.

**Step 4: Run test to verify it passes**

Run the same test again.

Expected: PASS.

**Step 5: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Private/Player/PvPArenaCharacter.cpp Source/PvPArena/Private/Tests/RangedAttackTimingTest.cpp
git -C /mnt/c/UE5CPP/PvPArena commit -m "fix: fire ranged projectile only from release notify"
```

### Task 6: Add camera aim offset state over the existing camera

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`
- Add or Modify test: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RangedAttackTimingTest.cpp`

**Step 1: Write the failing test**

Add assertions for:

- charging drives camera offset alpha away from default
- release/cancel returns camera offset alpha toward default

**Step 2: Run test to verify it fails**

Run:

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Combat.RangedAttackTiming; Quit'"
```

Expected: FAIL because no camera offset state exists yet.

**Step 3: Write minimal implementation**

Implement:

- cached original camera offset
- configurable target right offset
- tick-time interpolation during charge and back to default after exit
- safe no-op if expected camera owner cannot be found

**Step 4: Run test to verify it passes**

Run the same test again.

Expected: PASS.

**Step 5: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Public/Player/PvPArenaCharacter.h Source/PvPArena/Private/Player/PvPArenaCharacter.cpp Source/PvPArena/Private/Tests/RangedAttackTimingTest.cpp
git -C /mnt/c/UE5CPP/PvPArena commit -m "feat: add ranged charge camera offset blending"
```

### Task 7: Verify HUD and projectile defaults still behave after charge conversion

**Files:**
- Modify only if needed: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`
- Modify only if needed: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RangedProjectileDefaultsTest.cpp`
- Modify only if needed: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`

**Step 1: Write the failing test**

Only add tests if regressions appear:

- HUD cooldown display still counts down after successful release
- projectile defaults remain unchanged

**Step 2: Run tests to verify current status**

Run:

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Combat.CooldownValidation+PvPArena.Combat.RangedProjectileDefaults+PvPArena.UI.HUDWidgetLayout; Quit'"
```

Expected: PASS, or targeted failures that drive minimal fixes.

**Step 3: Write minimal implementation**

Apply only the smallest fixes required by the failing tests.

**Step 4: Run tests to verify they pass**

Run the same test command again.

Expected: PASS.

**Step 5: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp Source/PvPArena/Private/Tests/RangedProjectileDefaultsTest.cpp Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp
git -C /mnt/c/UE5CPP/PvPArena commit -m "test: verify hud and projectile behavior after ranged charge conversion"
```

### Task 8: Final verification

**Files:**
- No intended code changes

**Step 1: Run focused automation suite**

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Combat.RangedAttackTiming+PvPArena.Combat.CooldownValidation+PvPArena.Combat.RangedProjectileDefaults+PvPArena.UI.HUDWidgetLayout; Quit'"
```

Expected: PASS for all targeted tests.

**Step 2: Run build**

```bash
powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat' PvPArenaEditor Win64 Development '-Project=C:\UE5CPP\PvPArena\PvPArena.uproject' -WaitMutex -NoHotReloadFromIDE"
```

Expected: BUILD SUCCESSFUL with exit code `0`.

**Step 3: Manual editor verification**

Check in PIE:

- hold right-click under 1.0 seconds -> cancel, no projectile, no cooldown
- hold right-click over 1.0 seconds -> release montage, projectile from release notify, cooldown starts
- while holding, camera shifts right smoothly
- after release/cancel, camera returns smoothly
- hold loop does not visibly snap

**Step 4: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena status --short
git -C /mnt/c/UE5CPP/PvPArena commit -am "feat: add hold to charge ranged attack flow"
```
