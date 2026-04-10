# Texture Budget Reduction Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Reduce cooked texture size and runtime texture memory in `PvPArena` by auditing arena textures, normalizing asset settings, and validating changes against the primary arena map.

**Architecture:** This pass focuses on asset-level texture budget control instead of material-system redesign. The workflow establishes a baseline, classifies active arena textures by visual importance, applies conservative but meaningful resolution and streaming changes, and then verifies both package-size and runtime-memory impact on a fixed map.

**Tech Stack:** Unreal Engine 5 editor asset settings, project config (`.ini`), cooked asset/package verification, texture streaming diagnostics.

---

### Task 1: Capture the current texture baseline

**Files:**
- Create: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/docs/plans/2026-04-10-texture-budget-baseline.md`
- Verify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Config/DefaultEngine.ini`
- Verify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Config/DefaultScalability.ini`

**Step 1: Record the current baseline**

Document:

- current streaming pool settings
- target validation map: `PvPArena_map`
- the initial list of candidate textures under `Content/PvPArena/Textures` and `Content/PvPArena/Textures/Trim_Textures`
- any known `Never Stream`, oversized, or obviously duplicated texture assets

**Step 2: Run baseline verification**

Capture:

- texture streaming diagnostics on `PvPArena_map`
- current cooked or estimated texture sizes for the target set
- visual reference screenshots for hero surfaces and trims

**Step 3: Save the baseline notes**

Store the measured before-state in the baseline markdown file so the pass has a stable comparison point.

### Task 2: Classify active arena textures by importance

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/docs/plans/2026-04-10-texture-budget-baseline.md`
- Verify through editor content browser:
  - `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures`
  - `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures/Trim_Textures`

**Step 1: Build the classification list**

Assign each targeted texture to one of:

- `Hero`
- `Gameplay Visible`
- `Background/Trim`
- `Mask/ORM/Noise`

**Step 2: Mark action candidates**

For each asset, note one of:

- keep
- reduce one tier
- reduce aggressively
- inspect for duplication/packing cleanup

**Step 3: Verify the classification is map-driven**

Confirm the list reflects actual use in `PvPArena_map` rather than raw folder contents alone.

### Task 3: Apply low-risk texture setting reductions

**Files:**
- Modify targeted assets in: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures`
- Modify targeted assets in: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures/Trim_Textures`

**Step 1: Reduce secondary textures first**

Apply conservative reductions to:

- `Background/Trim` base color textures
- non-hero normal maps
- mask, imperfection, and grayscale utility textures

Use asset settings such as:

- lower imported max size where appropriate
- reduced `Max Texture Size`
- correct compression settings for masks and normals
- enabled streaming unless a texture has a documented reason not to stream

**Step 2: Recheck visual quality on the arena map**

Open `PvPArena_map` and inspect representative walls, trims, and surfaces for:

- obvious blur
- mip popping
- broken roughness/metal response
- normal-map artifacts

**Step 3: Keep the pass incremental**

Save changes in small batches so any visual regression can be isolated quickly.

### Task 4: Normalize packed and utility texture usage

**Files:**
- Modify targeted assets in: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures`
- Modify targeted assets in: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures/Trim_Textures`
- Verify related material consumers in `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Materials`

**Step 1: Audit grayscale and packed textures**

Identify:

- standalone roughness, AO, metallic, or mask textures that may already be represented in packed maps
- duplicate trim texture variants with effectively redundant usage

**Step 2: Apply only safe consolidation**

Remove or replace redundant utility textures only when the consuming material path is clear and behavior stays identical.

**Step 3: Revalidate materials**

Verify the affected materials in-editor to ensure packed channel assumptions still match the asset settings.

### Task 5: Add project-level guardrails only if needed

**Files:**
- Modify if necessary: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Config/DefaultEngine.ini`
- Modify if necessary: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Config/DefaultScalability.ini`

**Step 1: Decide whether config changes are justified**

Only add config-level texture rules if asset-level cleanup alone does not produce a stable budget or if repeated asset classes clearly need a shared policy.

**Step 2: Keep config changes narrow**

If changes are needed, prefer:

- clearly scoped texture group adjustments
- comments or notes in the baseline doc describing why the shared rule exists

Avoid using a larger streaming pool as the primary fix.

**Step 3: Verify config impact**

Re-run arena-map checks to confirm the shared policy helps without over-degrading important surfaces.

### Task 6: Verify package-size and runtime-memory results

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/docs/plans/2026-04-10-texture-budget-baseline.md`

**Step 1: Run after-state verification**

Capture:

- updated texture streaming diagnostics on `PvPArena_map`
- before/after size comparison for the changed texture set
- packaged or cooked size deltas when available

**Step 2: Summarize regressions and holdouts**

Document:

- which textures changed
- which textures were intentionally kept at higher quality
- any remaining oversized assets deferred to a later pass

**Step 3: Declare completion only with evidence**

Do not treat the pass as complete until the recorded after-state shows improvement in both:

- runtime texture budget pressure
- cooked/package texture footprint
