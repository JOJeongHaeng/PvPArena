# Texture Budget Next Phase Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Resolve the remaining high-risk texture ambiguity, design a small third reduction batch, and record after-state evidence for the ongoing texture budget pass.

**Architecture:** First isolate and document the truth about `T_Concrete_2_N` so the pass stops carrying an unresolved large texture candidate. Then define a narrow `Batch 3` from low-risk utility and trim assets, apply it through the same Unreal Python workflow used by the earlier batches, and finish by updating the baseline evidence with concrete before and after results.

**Tech Stack:** Unreal Engine 5.5 editor asset settings, Unreal Python editor scripting, markdown planning docs, asset reference inspection via CLI and editor.

---

### Task 1: Lock down the `T_Concrete_2_N` investigation

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/docs/plans/2026-04-10-texture-budget-baseline.md`
- Verify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures/T_Concrete_2_N.uasset`
- Verify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Materials`

**Step 1: Record the current asset metadata**

Capture in the baseline doc:

- current imported format
- compression setting
- texture group
- `Max Texture Size`
- any unusual evidence that distinguishes `T_Concrete_2_N` from normal arena normal maps

**Step 2: Confirm reference evidence**

Run targeted CLI reference checks and, if needed, confirm through the editor whether `T_Concrete_2_N` is actively consumed by an arena material or instance.

Expected: a clear statement in the baseline doc that the asset is either actively used, weakly referenced, or still unconfirmed.

**Step 3: Write the disposition**

Add one explicit disposition to the baseline doc:

- `keep`
- `cap only`
- `format-fix candidate`
- `unused or deferred`

Expected: no ambiguous wording remains around `T_Concrete_2_N`.

### Task 2: Define the narrow `Batch 3` candidate set

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/docs/plans/2026-04-10-texture-budget-baseline.md`
- Create: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Scripts/texture_budget_batch_3.py`
- Verify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures`
- Verify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures/Trim_Textures`

**Step 1: Choose 3 to 5 low-risk assets**

Prioritize:

- ORM maps
- AO or imperfection masks
- trim textures
- non-hero support textures

Avoid:

- broad hero surface normals
- large visual identity textures unless the investigation proves they are safe

Expected: baseline doc lists the selected `Batch 3` assets and the reason each one is included.

**Step 2: Define the target caps**

Assign a concrete `Max Texture Size` to each selected asset based on the earlier batch rules.

Expected: the baseline doc shows the exact cap planned for each `Batch 3` asset.

**Step 3: Write the automation script**

Create `Scripts/texture_budget_batch_3.py` following the same pattern as Batch 1 and Batch 2:

- load each asset by content path
- change only `max_texture_size`
- save only when a value actually changes
- log the changed assets

Expected: the script compiles cleanly with `python3 -m py_compile`.

### Task 3: Apply and verify `Batch 3`

**Files:**
- Modify targeted assets under `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures`
- Modify targeted assets under `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures/Trim_Textures`
- Verify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Scripts/texture_budget_batch_3.py`

**Step 1: Run the script through `UnrealEditor-Cmd`**

Use the existing worktree-local project and the same headless flags already validated for Batch 1 and Batch 2.

Expected: Unreal logs per-asset property changes and saves all selected packages.

**Step 2: Record what changed**

Capture which assets actually moved from `0` or an older value to the new cap.

Expected: a stable asset list exists for the after-state notes.

**Step 3: Perform the immediate sanity check**

Note whether the batch completed cleanly or whether any asset failed to load or save.

Expected: no silent failures remain hidden before the documentation pass.

### Task 4: Update the after-state evidence

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/docs/plans/2026-04-10-texture-budget-baseline.md`

**Step 1: Add the `Batch 3` section**

Document:

- selected assets
- classifications
- applied caps
- execution notes

Expected: the baseline doc reads as a continuous log from Batch 1 through Batch 3.

**Step 2: Add before and after size evidence**

Record:

- per-asset or grouped size deltas for the changed assets
- any cooked or package-size evidence that is available

Expected: the doc includes a concrete size comparison instead of only intent.

**Step 3: Add runtime validation notes**

Record:

- user visual feedback on `PvPArena_map`
- any texture streaming diagnostics available in this environment
- which assets remain deferred and why

Expected: the after-state section makes it clear what improved and what is still pending.

### Task 5: Verify and prepare handoff

**Files:**
- Verify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/docs/plans/2026-04-10-texture-budget-baseline.md`
- Verify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/docs/plans/2026-04-10-texture-budget-next-phase.md`
- Verify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Scripts/texture_budget_batch_3.py`

**Step 1: Re-run local verification**

Run:

- `python3 -m py_compile /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Scripts/texture_budget_batch_3.py`
- `git -C /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction diff -- docs/plans/2026-04-10-texture-budget-baseline.md /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/docs/plans/2026-04-10-texture-budget-next-phase.md /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Scripts/texture_budget_batch_3.py`

Expected: the script syntax is valid and the diffs match the intended scope.

**Step 2: Summarize remaining holdouts**

Explicitly list any assets still deferred after Batch 3.

Expected: the handoff makes the next pass obvious without reopening old ambiguity.

**Step 3: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction add docs/plans/2026-04-10-texture-budget-reduction-design.md docs/plans/2026-04-10-texture-budget-next-phase.md docs/plans/2026-04-10-texture-budget-baseline.md Scripts/texture_budget_batch_3.py
git -C /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction commit -m "docs: plan next texture budget phase"
```

Expected: the next-phase plan and supporting notes are captured as a clean worktree-local change set.
