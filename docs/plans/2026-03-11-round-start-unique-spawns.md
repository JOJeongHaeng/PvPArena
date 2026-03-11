# Round Start Unique Spawns Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Ensure each new round starts players on different `PlayerStart` actors whenever multiple starts exist.

**Architecture:** Keep the existing respawn path for ordinary eliminations, but add a round-reset-specific exclusion set so `ResetAllPlayersForNextRound()` never assigns the same start twice in the same reset pass unless there is no alternative. Reuse the existing spawn-selection helpers instead of inventing a second spawn system.

**Tech Stack:** Unreal Engine 5.5 C++, Unreal Automation Tests

---

### Task 1: Add failing round-reset uniqueness coverage

**Files:**
- Modify: `Source/PvPArena/Private/Tests/RandomRespawnStartSelectionTest.cpp`
- Modify: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Test: `Source/PvPArena/Private/Tests/RandomRespawnStartSelectionTest.cpp`

**Step 1: Write the failing test**

Add a focused test that simulates a round-start pass for two controllers and asserts the second controller cannot reuse the start already assigned to the first controller when another candidate exists.

**Step 2: Run test to verify it fails**

Run the editor build and focused automation test.
Expected: FAIL because the round-start exclusion helper does not exist yet.

**Step 3: Write minimal implementation**

Expose a helper that selects a round-start spawn from a candidate list while excluding starts already used in the current reset pass.

**Step 4: Run test to verify it passes**

Run the focused automation test again.
Expected: PASS

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Game/PvPArenaGameMode.h Source/PvPArena/Private/Tests/RandomRespawnStartSelectionTest.cpp
git commit -m "test: cover unique round start spawns"
```

### Task 2: Apply unique selection during round resets

**Files:**
- Modify: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Modify: `Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`
- Test: `Source/PvPArena/Private/Tests/RandomRespawnStartSelectionTest.cpp`

**Step 1: Keep the new test as the guardrail**

Use the new failing test to drive the implementation.

**Step 2: Write minimal runtime implementation**

- track starts used during a single `ResetAllPlayersForNextRound()` pass
- choose a unique start for each controller when alternatives exist
- preserve fallback behavior when unique choices run out

**Step 3: Run test to verify it still passes**

Run the focused automation test again.

**Step 4: Run build verification**

Run the editor build for the worktree.

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Game/PvPArenaGameMode.h Source/PvPArena/Private/Game/PvPArenaGameMode.cpp Source/PvPArena/Private/Tests/RandomRespawnStartSelectionTest.cpp
git commit -m "fix: avoid duplicate round start spawns"
```

### Task 3: Verify and document

**Files:**
- Modify: `docs/plans/next-session.md`

**Step 1: Run focused automation tests**

Run:
`UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests PvPArena.Match.RandomRespawnStartSelection; Quit"`

Expected: PASS

**Step 2: Update handoff**

Record that new rounds now distribute active players across distinct starts when multiple `PlayerStart` actors are available.

**Step 3: Commit**

```bash
git add docs/plans/next-session.md docs/plans/2026-03-11-round-start-unique-spawns-design.md docs/plans/2026-03-11-round-start-unique-spawns.md
git commit -m "docs: record unique round start spawn rule"
```
