# Ranged Projectile Conversion Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Convert right-click ranged attack from notify-timed hitscan into a server-authoritative straight-line projectile that disappears on wall or target impact.

**Architecture:** Keep the existing notify-driven ranged attack timing, but replace the notify hit execution with projectile spawn. Add a lightweight projectile actor with sphere collision and projectile movement, then route server damage through projectile hit handling.

**Tech Stack:** Unreal Engine 5 C++, AActor, USphereComponent, UProjectileMovementComponent, Automation Tests

---

### Task 1: Add failing projectile tests

**Files:**
- Create: `Source/PvPArena/Private/Tests/RangedProjectileDefaultsTest.cpp`
- Modify: `Source/PvPArena/Private/Tests/RangedAttackTimingTest.cpp`

**Step 1: Write failing tests**
- Assert ranged attack spawns a projectile class instead of applying instant hitscan logic.
- Assert the projectile defaults to no gravity and collision-enabled movement.

**Step 2: Run tests to verify failure**

### Task 2: Add ranged projectile actor

**Files:**
- Create: `Source/PvPArena/Public/Combat/PvPProjectile.h`
- Create: `Source/PvPArena/Private/Combat/PvPProjectile.cpp`
- Modify: `Source/PvPArena/PvPArena.Build.cs`

**Step 1: Implement projectile actor**
- Sphere collision
- Projectile movement
- No gravity
- Ignore owner/instigator
- Destroy on blocking hit

### Task 3: Replace ranged hitscan with projectile spawn

**Files:**
- Modify: `Source/PvPArena/Public/Combat/PvPCombatComponent.h`
- Modify: `Source/PvPArena/Private/Combat/PvPCombatComponent.cpp`
- Modify: `Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`

**Step 1: Spawn projectile on ranged notify**
- Use locked ranged yaw
- Spawn from view location or a small forward offset
- Remove direct ranged line trace damage path

### Task 4: Verify

**Step 1: Run relevant automation**
- `PvPArena.Combat.RangedAttackTiming`
- `PvPArena.Combat.CooldownValidation`
- new projectile defaults test

**Step 2: Manual gameplay check**
- Projectile flies straight
- Disappears on wall impact
- Damages enemy on hit
