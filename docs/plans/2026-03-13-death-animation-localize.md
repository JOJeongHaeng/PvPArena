# Death Animation Localization Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Point the default character death animation at a project-owned asset instead of the imported mocap pack path.

**Architecture:** Keep the current `APvPArenaCharacter` death animation behavior intact and change only the default asset reference plus the copied asset dependency inside the feature worktree. Verify the existing regression test still covers the behavior and rebuild the editor target after the change.

**Tech Stack:** Unreal Engine 5.5 C++, Unreal content assets, Unreal Automation Tests

---

### Task 1: Bring the duplicated death animation asset into the worktree

**Files:**
- Copy: `Content/PvPArena/Animations/Stand_Relaxed_Death.uasset`

**Step 1: Confirm the source asset exists in the root project**

Run:

```bash
find /mnt/c/UE5CPP/PvPArena/Content/PvPArena/Animations -maxdepth 1 -type f
```

Expected: `Stand_Relaxed_Death.uasset` is present.

**Step 2: Copy the asset into the feature worktree**

Copy the asset from the root project into:

```text
/mnt/c/UE5CPP/PvPArena/.worktrees/feature-death-animation-localize/Content/PvPArena/Animations/Stand_Relaxed_Death.uasset
```

**Step 3: Verify the worktree sees the copied asset**

Run:

```bash
find /mnt/c/UE5CPP/PvPArena/.worktrees/feature-death-animation-localize/Content/PvPArena/Animations -maxdepth 1 -type f
```

Expected: the copied asset appears in the worktree.

### Task 2: Update the default death animation reference with TDD

**Files:**
- Modify: `Source/PvPArena/Private/Tests/CharacterDeathAnimationDefaultsTest.cpp`
- Modify: `Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`

**Step 1: Write the failing test**

Extend `CharacterDeathAnimationDefaultsTest` so it asserts the configured default asset path resolves to the project-owned death animation asset rather than only checking for non-null.

**Step 2: Run the focused test to verify it fails**

Run:

```bash
UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests PvPArena.Character.DeathAnimationDefaults; Quit"
```

Expected: FAIL because the code still points at `/Game/MCO_Mocap_Basics/...`.

**Step 3: Write the minimal implementation**

Update the `ConstructorHelpers::FObjectFinder<UAnimationAsset>` path in `APvPArenaCharacter` to:

```text
/Game/PvPArena/Animations/Stand_Relaxed_Death.Stand_Relaxed_Death
```

**Step 4: Re-run the focused test to verify it passes**

Run the same `PvPArena.Character.DeathAnimationDefaults` automation test.

Expected: PASS.

**Step 5: Commit**

```bash
git add Content/PvPArena/Animations/Stand_Relaxed_Death.uasset Source/PvPArena/Private/Player/PvPArenaCharacter.cpp Source/PvPArena/Private/Tests/CharacterDeathAnimationDefaultsTest.cpp
git commit -m "fix: localize death animation asset reference"
```

### Task 3: Verify build health and document outcome

**Files:**
- Optional review: `docs/plans/next-session.md`

**Step 1: Run the editor build**

Run the Unreal editor build for the feature worktree.

Expected: build succeeds.

**Step 2: Review branch diff**

Run:

```bash
git -C /mnt/c/UE5CPP/PvPArena/.worktrees/feature-death-animation-localize diff --stat main...HEAD
```

Expected: only the copied animation asset, code/test change, and the planning docs appear.

**Step 3: Leave handoff unchanged unless new follow-up is discovered**

Do not update broader handoff docs unless the localized animation path creates a new requirement for future sessions.
