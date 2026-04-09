# Runtime Memory And Performance Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Reduce runtime overhead and memory pressure in packaged builds by removing unnecessary per-frame work from projectiles and HUD widgets without changing combat behavior.

**Architecture:** This pass avoids risky combat-asset loading changes and instead removes wasteful ticking paths that stay active across gameplay. Projectile debug rendering becomes opt-in, HUD crosshair refresh relies on its existing polling loop, and overhead nameplates move from frame-tick refresh to timer-driven refresh.

**Tech Stack:** Unreal Engine 5 C++, UMG widgets, actor tick control, `FTimerManager`, automation tests.

---

### Task 1: Lock in low-risk optimization expectations with tests

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RangedProjectileDefaultsTest.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetRangedCrosshairTest.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/OverheadStatusWidgetTest.cpp`

**Step 1: Write the failing test**

Add assertions for:

- projectile actor ticking is disabled by default when debug drawing is off
- HUD crosshair refresh behavior is still reachable through `RefreshWidgetData()`
- overhead widget refresh does not require `NativeTick()` to update once constructed

**Step 2: Run test to verify it fails**

Run the focused automation tests for projectile, HUD widget, and overhead widget behavior.

**Step 3: Write minimal implementation**

Update the runtime refresh paths and tick defaults to satisfy the new expectations.

**Step 4: Run test to verify it passes**

Run the same focused automation tests again.

### Task 2: Remove unnecessary projectile ticking

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Combat/PvPProjectile.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Combat/PvPProjectile.h`
- Verify through: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RangedProjectileDefaultsTest.cpp`

**Step 1: Write the failing test**

Assert that a new projectile starts with ticking disabled when debug drawing is not requested.

**Step 2: Run test to verify it fails**

Run the projectile test.

**Step 3: Write minimal implementation**

Make projectile ticking opt-in for debug visualization only, preserving existing hit, replication, and lifespan behavior.

**Step 4: Run test to verify it passes**

Run the projectile test again.

### Task 3: Remove HUD frame tick refresh

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Verify through: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetRangedCrosshairTest.cpp`

**Step 1: Write the failing test**

Assert that `RefreshWidgetData()` updates crosshair visibility without requiring a per-frame widget tick.

**Step 2: Run test to verify it fails**

Run the HUD widget test.

**Step 3: Write minimal implementation**

Remove the `NativeTick` override and fold crosshair visibility refresh into the existing timer-driven path only.

**Step 4: Run test to verify it passes**

Run the HUD widget test again.

### Task 4: Replace overhead nameplate frame tick with timer refresh

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/UI/PvPArenaOverheadStatusWidget.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/UI/PvPArenaOverheadStatusWidget.h`
- Verify through: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/OverheadStatusWidgetTest.cpp`

**Step 1: Write the failing test**

Assert that the overhead widget can refresh after construction without relying on `NativeTick`, and that cleanup clears any repeating refresh timer.

**Step 2: Run test to verify it fails**

Run the overhead widget test.

**Step 3: Write minimal implementation**

Replace `NativeTick` with a repeating timer in `NativeConstruct`, clear it in `NativeDestruct`, and keep `RefreshFromOwningCharacter()` as the single refresh entry point.

**Step 4: Run test to verify it passes**

Run the overhead widget test again.

### Task 5: Verify build and focused automation

**Files:**
- Verify only

**Step 1: Run verification**

Run:

- project build
- focused projectile automation
- focused HUD widget automation
- focused overhead widget automation

**Step 2: Summarize outcome**

Report:

- files changed
- which ticking paths were removed or reduced
- any remaining high-risk memory optimization candidates deferred from this pass
