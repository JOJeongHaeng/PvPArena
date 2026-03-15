# Runtime Animation Localization Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Remove runtime dependence on external animation-pack assets and reintroduce melee animation using project-owned assets only.

**Architecture:** First identify which current runtime paths still resolve through external packs or broken localized copies. Then replace those runtime references with project-owned assets under `Content/PvPArena/Animations`, updating code and blueprint defaults only after the assets are valid on the project mannequin skeleton. Verification stays focused on build, automation, and targeted PIE checks.

**Tech Stack:** Unreal Engine 5.5 C++, Blueprint defaults, Animation Sequences, Animation Montages, Automation Tests

---

### Task 1: Inventory current runtime animation dependencies

**Files:**
- Inspect: `Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`
- Inspect: `Source/PvPArena/Private/Tests/CharacterDeathAnimationDefaultsTest.cpp`
- Inspect: `Content/PvPArena/BP_PvPArenaCharacter.uasset`
- Inspect: `Content/PvPArena/Animations/Stand_Relaxed_Death.uasset`

**Step 1: Gather current code-path animation references**

Run:

```bash
rg -n "Stand_Relaxed_Death|DeathAnimation|MeleeAttackMontage|FreeAnimationLibrary|MCO_Mocap_Basics" Source/PvPArena
```

Expected: explicit runtime code paths are visible before asset edits begin.

**Step 2: Reproduce current asset load warnings**

Run the focused death animation automation test:

```bash
UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests PvPArena.Character.DeathAnimationDefaults; Quit"
```

Expected: current log output shows whether `Stand_Relaxed_Death` still depends on missing external packages.

**Step 3: Inspect character blueprint defaults in the editor**

Check whether `BP_PvPArenaCharacter` overrides mesh, anim class, or animation asset references with external-pack paths.

Expected: a clear list of blueprint-owned runtime references that also need localization.

### Task 2: Re-localize the death animation path

**Files:**
- Modify: `Content/PvPArena/Animations/Stand_Relaxed_Death.uasset`
- Verify: `Source/PvPArena/Private/Tests/CharacterDeathAnimationDefaultsTest.cpp`

**Step 1: Create a valid project-owned death animation asset**

In the editor, create or retarget a death animation under:

```text
/Game/PvPArena/Animations/Stand_Relaxed_Death
```

using the project mannequin skeleton and a valid preview mesh.

Expected: the animation opens without A-pose or missing skeleton warnings.

**Step 2: Re-save and validate the localized death asset**

Open the asset directly and verify:
- skeleton is project-owned mannequin skeleton
- preview mesh resolves
- playback is valid

Expected: no dependency on `MCO_Mocap_Basics` or other missing packs remains for runtime use.

**Step 3: Re-run the focused death animation test**

Run:

```bash
UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests PvPArena.Character.DeathAnimationDefaults; Quit"
```

Expected: PASS, with no missing skeleton/package warnings for the localized death asset.

### Task 3: Localize melee attack assets correctly

**Files:**
- Create: `Content/PvPArena/Animations/anim_Counter_Attack_01.uasset`
- Create: `Content/PvPArena/Animations/MTG_MeleeAttack01.uasset`
- Modify: `Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`
- Modify: `Source/PvPArena/Private/Tests/CombatCooldownValidationTest.cpp`
- Create: `Source/PvPArena/Private/Tests/MeleeAttackTimingTest.cpp`
- Create: `Source/PvPArena/Public/Animation/PvPAnimNotify_MeleeHit.h`
- Create: `Source/PvPArena/Private/Animation/PvPAnimNotify_MeleeHit.cpp`

**Step 1: Write or restore focused melee timing tests**

Ensure the melee timing test asserts:
- attack enters in-progress state
- hit execution is notify-driven
- repeated starts are blocked during the active swing

Expected: test captures the intended runtime behavior before implementation changes.

**Step 2: Create a valid project-owned melee animation**

In the editor, retarget or otherwise create a valid attack animation using the project mannequin skeleton and save it to:

```text
/Game/PvPArena/Animations/anim_Counter_Attack_01
```

Expected: the sequence opens and plays normally in the worktree project.

**Step 3: Create the montage and notify**

Create:

```text
/Game/PvPArena/Animations/MTG_MeleeAttack01
```

with exactly one impact notify, preferably `PvPAnimNotify_MeleeHit`.

Expected: runtime can load and play the montage without A-pose or missing dependency warnings.

**Step 4: Implement minimal notify-timed melee flow**

Wire character code so accepted melee input starts the montage, the notify applies the hit exactly once, and montage end clears in-progress state.

Expected: runtime references only `/Game/PvPArena/Animations/...` paths.

### Task 4: Verify runtime localization end-to-end

**Files:**
- Verify: animation assets above
- Verify: `Content/PvPArena/BP_PvPArenaCharacter.uasset`
- Verify: code and tests above

**Step 1: Build the editor target**

Run:

```bash
Build.bat PvPArenaEditor Win64 Development -Project="C:\UE5CPP\PvPArena\.worktrees\feature-melee-attack-animation\PvPArena.uproject" -WaitMutex -NoHotReloadFromIDE
```

Expected: build succeeds.

**Step 2: Run focused automation coverage**

Run:

```bash
Automation RunTests PvPArena.Character.DeathAnimationDefaults
Automation RunTests PvPArena.Combat.MeleeAttackTiming
Automation RunTests PvPArena.Combat.CooldownValidation
```

Expected: all pass.

**Step 3: Perform targeted PIE validation**

Check:
- death animation still plays correctly
- left click plays the melee montage
- melee hit lands only on notify
- no A-pose occurs during either path

Expected: runtime animation paths are fully localized and stable.
