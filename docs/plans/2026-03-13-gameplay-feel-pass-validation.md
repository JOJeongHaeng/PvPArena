# Gameplay Feel Pass Validation Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Validate the merged gameplay tempo and spawn-selection rules in PIE, then make only the smallest code or map updates needed if the feel pass exposes a concrete issue.

**Architecture:** Start from the isolated `feature/gameplay-feel-pass` worktree and treat PIE validation as the source of truth. Use the existing test map and current short iteration defaults to check respawn pacing, invulnerability feel, round-end downtime, respawn variation, and round-start distribution. Only if a concrete issue is observed should the branch move into a small corrective change, followed by focused automation reruns and written validation notes.

**Tech Stack:** Unreal Engine 5.5 C++, Unreal Editor PIE, Unreal Automation Tests, Markdown docs, map assets under `Content/__ExternalActors__`

---

### Task 1: Reconfirm the branch baseline before PIE

**Files:**
- Review: `docs/plans/next-session.md`
- Review: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Review: `Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`
- Review: `Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp`
- Review: `Source/PvPArena/Private/Tests/RandomRespawnStartSelectionTest.cpp`

**Step 1: Review the active handoff and merged gameplay rules**

Read the handoff and the current game mode/test files so the validation uses the intended behavior:
- `RespawnDelaySeconds = 2`
- `RespawnInvulnerabilitySeconds = 1.25f`
- `RoundEndDelaySeconds = 3`
- respawns should vary across multiple `PlayerStart` actors
- immediate reuse of the previous start should be avoided when alternatives exist
- round resets should distribute players across unique starts when possible

**Step 2: Confirm the branch starts clean**

Run:

```bash
git -C /mnt/c/UE5CPP/PvPArena/.worktrees/feature-gameplay-feel-pass status --short
```

Expected: no uncommitted changes beyond the design/plan docs created for this branch.

**Step 3: Record the validation target**

Write down the specific PIE checks to perform so the pass does not drift into unrelated gameplay work.

### Task 2: Run the focused automated baseline

**Files:**
- Test: `Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp`
- Test: `Source/PvPArena/Private/Tests/RandomRespawnStartSelectionTest.cpp`
- Test: `Source/PvPArena/Private/Tests/EliminationRespawnBoundaryTest.cpp`

**Step 1: Build the editor target for the worktree**

Run the approved Unreal build command for this worktree.
Expected: build succeeds before manual validation starts.

**Step 2: Run the focused automation coverage**

Run the gameplay tests that back the feel pass:
- `PvPArena.Match.GameplayTempoDefaults`
- `PvPArena.Match.RandomRespawnStartSelection`
- `PvPArena.Match.EliminationRespawnBoundary`

Expected: PASS for all targeted tests.

**Step 3: Stop and investigate only if the baseline fails**

If any of the focused tests fail before PIE, treat that as a blocking regression and debug it before making map or gameplay adjustments.

### Task 3: Perform the manual PIE gameplay feel pass

**Files:**
- Review/possibly modify: `Content/__ExternalActors__/PvPArena/Maps/PvPArena_TestMap/`
- Review: `Config/DefaultEngine.ini`

**Step 1: Open the test map used by the project defaults**

Use `/Game/PvPArena/Maps/PvPArena_TestMap` in the editor and verify the level still contains multiple `PlayerStart` actors for the validation.

**Step 2: Validate respawn pacing**

In PIE, eliminate a player and confirm:
- the respawn delay feels close to `2` seconds
- the player is protected long enough after spawn to avoid unfair immediate re-death

Expected: the current `2s` respawn plus `1.25s` invulnerability feels intentional, not sluggish or too forgiving.

**Step 3: Validate round-end downtime**

End a round and confirm the pause before the next round feels close to `3` seconds and the HUD/state transition remains readable.

Expected: round resolution is visible without overstaying.

**Step 4: Validate respawn start variation**

With repeated eliminations, confirm respawns move across different `PlayerStart` actors and do not obviously repeat the last-used start when alternatives exist.

Expected: consecutive respawns for the same player vary when the map offers multiple valid starts.

**Step 5: Validate unique round starts**

Start a new round with multiple active players and confirm they begin on different starts when the map has enough `PlayerStart` actors.

Expected: players do not stack onto the same start when unique alternatives exist.

### Task 4: Apply the smallest fix only if PIE reveals a problem

**Files:**
- Possibly modify: `Content/__ExternalActors__/PvPArena/Maps/PvPArena_TestMap/...`
- Possibly modify: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Possibly modify: `Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`
- Possibly modify: `Source/PvPArena/Private/Tests/RandomRespawnStartSelectionTest.cpp`
- Possibly modify: `Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp`

**Step 1: Classify the issue**

Decide whether the observed problem is:
- map/content driven
- gameplay-rule driven
- missing automation coverage

**Step 2: Make the smallest correction**

Apply only one of these corrective paths:
- adjust `PlayerStart` placement/count if the map layout prevents meaningful variation
- change the smallest game-mode logic needed if the spawn rule behaves incorrectly
- extend the narrowest relevant test if validation exposed a missing assertion

**Step 3: Re-run the directly affected checks**

If code changed, rebuild and re-run the focused automation tests.
If only map assets changed, repeat the relevant PIE scenario that previously failed.

Expected: the original observed problem is no longer reproducible.

**Step 4: Commit the corrective change**

Commit only if a real fix was required, using a message that matches the actual correction, for example:

```bash
git add <touched files>
git commit -m "fix: tune gameplay feel validation issue"
```

### Task 5: Record the validation outcome

**Files:**
- Create: `docs/plans/2026-03-13-gameplay-feel-pass-validation-notes.md`

**Step 1: Write the validation notes**

Capture:
- whether the baseline build/tests passed
- what was observed in PIE for respawn delay, invulnerability, and round-end delay
- whether respawn variation and unique round starts behaved correctly
- whether any code or map changes were required
- what should happen next before final default restoration

**Step 2: Review the branch diff**

Run:

```bash
git -C /mnt/c/UE5CPP/PvPArena/.worktrees/feature-gameplay-feel-pass diff --stat main...HEAD
```

Expected: only the design doc, plan doc, validation notes, and any intentional minimal fix appear.

**Step 3: Commit the validation notes**

Run:

```bash
git add docs/plans/2026-03-13-gameplay-feel-pass-validation.md docs/plans/2026-03-13-gameplay-feel-pass-validation-notes.md
git commit -m "docs: capture gameplay feel validation results"
```
