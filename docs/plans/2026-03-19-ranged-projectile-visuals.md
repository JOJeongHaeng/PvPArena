# Ranged Projectile Visuals Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a visible orb core, energy shell, and hit impact effect to the ranged projectile without changing its existing gameplay alignment.

**Architecture:** Keep the collision sphere as the projectile root, attach a mesh core and Niagara shell at zero offset, and spawn a separate Niagara impact effect on hit. Drive the change through a single defaults test first, then implement the minimum constructor and hit logic needed to satisfy it.

**Tech Stack:** Unreal Engine 5 C++, `UStaticMeshComponent`, `UNiagaraComponent`, `UNiagaraFunctionLibrary`, automation tests

---

### Task 1: Lock the new projectile defaults in a failing test

**Files:**
- Modify: `Source/PvPArena/Private/Tests/RangedProjectileDefaultsTest.cpp`

**Step 1: Write the failing test**

- Replace the old assertion that the projectile has no Niagara component.
- Assert that the projectile creates:
  - a `UStaticMeshComponent`
  - a `UNiagaraComponent`
  - a non-null impact effect default via reflected property access

**Step 2: Run test to verify it fails**

Run: automation for `PvPArena.Combat.RangedProjectileDefaults`
Expected: FAIL because the projectile does not yet create those visual defaults.

**Step 3: Write minimal implementation**

- No implementation in this task.

**Step 4: Run test to verify it still fails for the right reason**

Run: same automation target after build
Expected: FAIL on missing mesh/Niagara/impact defaults, not compile errors.

### Task 2: Add projectile visual components and defaults

**Files:**
- Modify: `Source/PvPArena/Public/Combat/PvPProjectile.h`
- Modify: `Source/PvPArena/Private/Combat/PvPProjectile.cpp`

**Step 1: Write the failing test**

- Covered by Task 1.

**Step 2: Run test to verify it fails**

- Covered by Task 1.

**Step 3: Write minimal implementation**

- Add a visible mesh component and Niagara component to the projectile class.
- Add a configurable impact Niagara system property.
- Load default assets in the constructor and attach visual components to the collision root at zero offset.
- Spawn the impact Niagara system in `HandleProjectileHit` before destroy.

**Step 4: Run test to verify it passes**

Run: build plus automation for `PvPArena.Combat.RangedProjectileDefaults`
Expected: PASS

### Task 3: Verify the gameplay path still builds cleanly

**Files:**
- Modify: none unless verification exposes an issue

**Step 1: Write the failing test**

- No new test. Verification only.

**Step 2: Run test to verify current behavior**

Run: project build
Expected: PASS

**Step 3: Write minimal implementation**

- None unless build or focused test reveals a regression.

**Step 4: Run test to verify it passes**

Run: `PvPArena.Combat.RangedProjectileDefaults`
Expected: PASS

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Combat/PvPProjectile.h \
        Source/PvPArena/Private/Combat/PvPProjectile.cpp \
        Source/PvPArena/Private/Tests/RangedProjectileDefaultsTest.cpp \
        docs/plans/2026-03-19-ranged-projectile-visuals-design.md \
        docs/plans/2026-03-19-ranged-projectile-visuals.md
git commit -m "feat: add ranged projectile visuals"
```
