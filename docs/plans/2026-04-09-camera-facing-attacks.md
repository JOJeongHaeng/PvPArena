# Camera-Facing Attacks Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Rotate the character toward camera yaw during both melee and ranged attacks while keeping movement input direction unchanged, and make melee attacks play faster.

**Architecture:** Reuse the existing tick-driven attack-facing update in `APvPArenaCharacter`, but make it attack-generic instead of ranged-only. Preserve the current combat state machine and notify timing. Increase melee responsiveness through the existing montage play-rate property.

**Tech Stack:** Unreal Engine 5 C++, AnimMontage playback, controller yaw tracking, automation tests.

---

### Task 1: Lock in failing attack-facing and melee speed expectations

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/MeleeAttackTimingTest.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RangedAttackTimingTest.cpp`

**Step 1: Write the failing test**

Add assertions for:

- melee attack play rate is higher than the old default
- melee attack rotates toward controller yaw while active
- ranged attack rotates toward controller yaw while active without attack movement suppression

**Step 2: Run test to verify it fails**

Run the focused automation tests for melee and ranged timing.

**Step 3: Write minimal implementation**

Update attack-facing code and melee play rate to satisfy the new expectations.

**Step 4: Run test to verify it passes**

Run the same focused automation tests again.

### Task 2: Generalize attack-facing update for melee and ranged

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`

**Step 1: Write the failing test**

Use the new melee/ranged rotation assertions from Task 1.

**Step 2: Run test to verify it fails**

Run the focused automation tests.

**Step 3: Write minimal implementation**

Make attack-facing:

- active when melee or ranged attack is in progress
- use controller yaw as the target
- preserve current ranged compatibility state

**Step 4: Run test to verify it passes**

Run the same tests again.

### Task 3: Increase melee attack speed

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Verify through: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/MeleeAttackTimingTest.cpp`

**Step 1: Write the failing test**

Assert that melee play rate is above the previous `2.0f` default.

**Step 2: Run test to verify it fails**

Run the melee timing test.

**Step 3: Write minimal implementation**

Raise `MeleeAttackPlayRate` to the new desired default.

**Step 4: Run test to verify it passes**

Run the melee timing test again.

### Task 4: Verify build and focused automation

**Files:**
- Verify only

**Step 1: Run verification**

Run:

- project build
- `PvPArena.Combat.MeleeAttackTiming`
- `PvPArena.Combat.RangedAttackTiming`

**Step 2: Summarize outcome**

Report:

- files changed
- what gameplay behavior changed
- any remaining editor-side animation adjustments
