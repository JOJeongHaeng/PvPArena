# Ranged Attack Notify HUD Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a right-click ranged attack that uses a delayed animation notify, suppresses movement during the attack, applies a 5 second cooldown, and exposes that cooldown on the HUD as a gauge.

**Architecture:** Mirror the existing melee flow with a separate ranged montage, begin/notify/finish state on the character, and server-authoritative hit execution on notify. Extend the combat component with ranged cooldown read APIs so the code-built HUD can render a progress bar and label from local character state.

**Tech Stack:** Unreal Engine 5 C++, AnimMontage, AnimNotify, code-built UMG, Automation Tests

---

### User-Owned Editor Tasks

**Files / Assets:**
- Create: `Content/PvPArena/Animations/MTG_RangedAttack_RightClick.uasset`
- Modify: `Content/PvPArena/Animations/AS_RangedAttack_RightClick.uasset`

**Step 1: Create the ranged montage**

- In Unreal Editor, create `MTG_RangedAttack_RightClick` from `AS_RangedAttack_RightClick`.

**Step 2: Add the ranged notify**

- Add a notify at the exact frame where the projectile / hit should occur.
- Prefer a dedicated notify class name rather than reusing the melee one.

**Step 3: Validate montage timing**

- Preview the montage and confirm the notify timing feels correct for the visible kick release.

### Task 1: Add failing ranged timing tests

**Files:**
- Create: `Source/PvPArena/Private/Tests/RangedAttackTimingTest.cpp`
- Modify: `Source/PvPArena/Private/Tests/CombatCooldownValidationTest.cpp`

**Step 1: Write the failing tests**

- Add a ranged timing automation test that expects:
  - ranged attack enters an in-progress state
  - attack start does not immediately fire
  - notify triggers the ranged hit once
  - finish clears ranged state and movement suppression
- Extend cooldown validation to assert ranged cooldown is 5.0 seconds and immediate reuse is blocked.

**Step 2: Run tests to verify they fail**

Run:
`UnrealEditor-Cmd.exe PvPArena.uproject -ExecCmds="Automation RunTests PvPArena.Combat.RangedAttackTiming; Automation RunTests PvPArena.Combat.CooldownValidation; Quit"`

Expected:
- new ranged timing test fails because ranged state/notify APIs do not exist yet
- cooldown test fails because ranged cooldown is still `1.2f`

### Task 2: Implement ranged attack state and montage flow

**Files:**
- Modify: `Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`
- Create: `Source/PvPArena/Public/Animation/PvPAnimNotify_RangedHit.h`
- Create: `Source/PvPArena/Private/Animation/PvPAnimNotify_RangedHit.cpp`

**Step 1: Write minimal implementation**

- Add ranged equivalents for:
  - in-progress flag
  - notify-triggered flag
  - begin / notify handler / trigger / finish methods
  - montage play / multicast / montage end handling
- Reuse movement suppression during ranged attack.
- Load `MTG_RangedAttack_RightClick` by default asset path.

**Step 2: Connect server flow**

- Change `ServerTryRangedAttack()` so it starts the ranged attack and plays the montage, but does not execute the hit immediately.
- Fire `TryServerRangedAttack(this)` only from the ranged notify path.

**Step 3: Run tests to verify they pass**

Run:
`UnrealEditor-Cmd.exe PvPArena.uproject -ExecCmds="Automation RunTests PvPArena.Combat.RangedAttackTiming; Automation RunTests PvPArena.Combat.CooldownValidation; Quit"`

Expected:
- both tests pass

### Task 3: Expose ranged cooldown state to UI

**Files:**
- Modify: `Source/PvPArena/Public/Combat/PvPCombatComponent.h`
- Modify: `Source/PvPArena/Private/Combat/PvPCombatComponent.cpp`

**Step 1: Write the failing test expectation**

- Update cooldown validation so it also checks remaining cooldown math / alpha after use.

**Step 2: Write minimal implementation**

- Set `RangedCooldownSeconds = 5.0f`.
- Add read APIs for:
  - total ranged cooldown
  - remaining ranged cooldown at a given time
  - normalized HUD alpha

**Step 3: Run tests**

Run:
`UnrealEditor-Cmd.exe PvPArena.uproject -ExecCmds="Automation RunTests PvPArena.Combat.CooldownValidation; Quit"`

Expected:
- pass with deterministic cooldown math

### Task 4: Add ranged cooldown gauge to HUD

**Files:**
- Modify: `Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Modify: `Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`

**Step 1: Write the failing UI test or helper-level assertion**

- Add or extend a small automation test around a HUD helper so ranged cooldown label / percentage can be validated without a full runtime widget interaction harness.

**Step 2: Write minimal implementation**

- Add a ranged cooldown `UProgressBar` and `UTextBlock` to the left status panel.
- Refresh from local pawn -> combat component every HUD tick.
- Show:
  - `Ranged: Ready` when available
  - `Ranged: X.Xs` while cooling down
- Render a gauge that depletes or fills consistently with the chosen cooldown math.

**Step 3: Run relevant tests**

Run:
`UnrealEditor-Cmd.exe PvPArena.uproject -ExecCmds="Automation RunTests PvPArena.UI; Automation RunTests PvPArena.Combat.CooldownValidation; Quit"`

Expected:
- HUD tests and cooldown validation pass

### Task 5: Final verification

**Files:**
- Verify: `Content/PvPArena/Animations/MTG_RangedAttack_RightClick.uasset`
- Verify: `Content/PvPArena/Animations/AS_RangedAttack_RightClick.uasset`
- Verify: `Source/PvPArena/...`

**Step 1: Manual gameplay verification**

- Launch editor/game and verify:
  - right click starts ranged montage
  - movement is blocked during the attack
  - hit occurs only on notify timing
  - cooldown is about 5 seconds
  - HUD gauge visibly counts down and returns to ready

**Step 2: Commit**

```bash
git add Content/PvPArena/Animations/MTG_RangedAttack_RightClick.uasset Source/PvPArena docs/plans/2026-03-16-ranged-attack-notify-hud.md
git commit -m "feat: add notify-driven ranged attack cooldown HUD"
```
