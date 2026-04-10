# Texture Budget Baseline

**Goal:** Capture the current texture budget starting point for `PvPArena` before any reduction pass.

**Source Notes**

- Config reference: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Config/DefaultEngine.ini`
- Streaming pool reference: `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Config/DefaultScalability.ini`
- Validation map: `PvPArena_map`
- Scope: `Content/PvPArena/Textures` and `Content/PvPArena/Textures/Trim_Textures`

## Current Config

- Both `DefaultEngine.ini` and `DefaultScalability.ini` currently set `r.Streaming.PoolSize=1000`.
- `DefaultEngine.ini` does not define any explicit texture LOD group policy in the current pass.
- There is no project-level budget note yet for hero versus trim versus utility texture classes.

## Target Scope

The first reduction pass should focus on textures that are actively part of the PvPArena presentation path:

- arena wall and floor materials
- trim sheets and modular panel textures
- prop textures used in the arena scene
- packed utility maps such as ORM, AO, and masks

Deferred for now:

- character texture sets
- UI textures
- broader starter or mannequin content outside the arena scope

## Current Folder Totals

- `Content/PvPArena/Textures`: `57` assets, about `470.33 MB` on disk
- `Content/PvPArena/Textures/Trim_Textures`: `28` assets, about `223.36 MB` on disk
- Combined first-pass texture set: `85` assets, about `693.69 MB` on disk

## Candidate Assets

These assets are the first classification candidates because their names suggest repeated use, packed utility data, or normal-map-heavy surfaces.

### Core Arena Surfaces

- `Content/PvPArena/Textures/T_GridMat_BC.uasset`
- `Content/PvPArena/Textures/T_GridMat_N.uasset`
- `Content/PvPArena/Textures/T_GridMat_ORM.uasset`
- `Content/PvPArena/Textures/T_Metal_1_N.uasset`
- `Content/PvPArena/Textures/T_Metal_1_ORM.uasset`
- `Content/PvPArena/Textures/T_Steel_1_B.uasset`
- `Content/PvPArena/Textures/T_Steel_1_N.uasset`
- `Content/PvPArena/Textures/T_Concrete_5_B.uasset`
- `Content/PvPArena/Textures/T_Concrete_5_N.uasset`
- `Content/PvPArena/Textures/T_Concrete_6_N.uasset`
- `Content/PvPArena/Textures/T_Concrete_Wall_1_A_N.uasset`
- `Content/PvPArena/Textures/T_Fence_1_B.uasset`
- `Content/PvPArena/Textures/T_Fence_1_N.uasset`

### Trim And Modular Sheet Candidates

- `Content/PvPArena/Textures/T_Trim_Metal_1_B.uasset`
- `Content/PvPArena/Textures/T_Trim_Metal_2_ORM.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_2_B.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_2_N.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_3_B.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_3_N.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_3_ORM.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_4_B.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_4_N.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_4_ORM.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_6_B.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_6_N.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_6_ORM.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_Trim7_BC.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_Trim7_N.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_Trim7_ORM.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Trim_Panel_2_N.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Trim_Pipe_1_B.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Trim_Pipe_1_N.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Trim_Pipe_1_ORM.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Wall_Trim_2_N.uasset`

### Prop Texture Candidates

- `Content/PvPArena/Textures/Props/T_Barrels_V1_Metal_BC.uasset`
- `Content/PvPArena/Textures/Props/T_Barrels_V1_Metal_N.uasset`
- `Content/PvPArena/Textures/Props/T_Barrels_V1_Metal_ORM.uasset`
- `Content/PvPArena/Textures/Props/T_Boxes_2_BC.uasset`
- `Content/PvPArena/Textures/Props/T_Boxes_2_N.uasset`
- `Content/PvPArena/Textures/Props/T_Boxes_2_ORM.uasset`
- `Content/PvPArena/Textures/Props/T_FuelTank_1_B.uasset`
- `Content/PvPArena/Textures/Props/T_FuelTank_1_N.uasset`
- `Content/PvPArena/Textures/Props/T_FuelTank_1_ORM.uasset`

### Highest Priority By On-Disk Size

The following assets were reported as the largest on-disk candidates and should be checked first during the classification pass:

- `Content/PvPArena/Textures/T_Concrete_5_B.uasset`
- `Content/PvPArena/Textures/T_GridMat_N.uasset`
- `Content/PvPArena/Textures/T_GridMat_ORM.uasset`
- `Content/PvPArena/Textures/T_Concrete_5_N.uasset`
- `Content/PvPArena/Textures/T_Concrete_2_N.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_3_B.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_3_ORM.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Trim_Pipe_1_ORM.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Props_B_ORM.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Props_A_ORM.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Wall_Trim_2_N.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_2_B.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_2_N.uasset`
- `Content/PvPArena/Textures/Trim_Textures/T_Panel_Trim7_N.uasset`
- Additional `Props/*` and `Trim_Textures/*` normal maps reported as large candidates should be reviewed in the same batch.

## Task 2 Classification Snapshot

This pass is a provisional Task 2 classification based on:

- target scope limited to `PvPArena_map` presentation textures
- asset family naming and folder structure
- binary string reference checks against `MaterialInstance` assets in `Content/PvPArena/Materials`

Editor-driven confirmation is still required for final keep/reduce decisions because this session cannot inspect viewport usage, streaming stats, or per-texture imported resolution directly.

### Family-Level Classification Rules For This Pass

- `Hero`
  - `T_GridMat_BC`, `T_GridMat_N`, and other arena-defining surface sets that appear to anchor repeated combat-space materials
  - default action: `keep`
- `Gameplay Visible`
  - concrete, fence, steel, and prop base textures that are likely visible during normal play but not close-up hero showcase surfaces
  - default action: `reduce one tier`
- `Background/Trim`
  - `Trim_Textures/T_Panel_*`, `Trim_Textures/T_Trim_*`, `Trim_Textures/T_Wall_Trim_*`, and other modular trim-sheet families
  - default action: `reduce one tier`, escalating to `reduce aggressively` for secondary normal maps
- `Mask/ORM/Noise`
  - `*_ORM`, `*_AO`, `*_R`, `*_M`, `*_E`, and similar packed or grayscale utility textures
  - default action: `reduce aggressively`, unless the texture is tied to a hero material where a one-tier drop is safer

### Priority Candidate Classification

| Asset | Reference clue | Classification | Action | Reasoning |
| --- | --- | --- | --- | --- |
| `T_Concrete_5_B` | `MI_Master_Concrete_1` | `Gameplay Visible` | `reduce one tier` | Concrete wall or floor color is likely common in the arena, but it is still a better reduction target than the main grid surface set. |
| `T_GridMat_N` | `MI_Fence_1` | `Hero` | `keep` | `GridMat` looks like a signature arena material family and its normal map is likely visible across broad combat-facing surfaces. |
| `T_GridMat_ORM` | `MI_Fence_1` | `Mask/ORM/Noise` | `reduce one tier` | It is a packed utility map, but because it appears attached to a repeated core surface, start conservatively. |
| `T_Concrete_5_N` | `MI_Master_Concrete_1` | `Gameplay Visible` | `reduce one tier` | Large normal map on a common surface; likely safe to trim by one step before touching hero-facing sets. |
| `T_Concrete_2_N` | `M_Master_Concrete_1` | `Gameplay Visible` | `inspect duplication/packing cleanup` | Reference traces suggest this may overlap with other concrete normal variants, so confirm whether it is active or redundant before reducing blindly. |
| `T_Panel_3_B` | `MI_Trim_Panel_3` | `Background/Trim` | `reduce one tier` | Panel trim-sheet base color is modular filler content and should be an early package-size target. |
| `T_Panel_3_ORM` | `MI_Trim_Panel_3` | `Mask/ORM/Noise` | `reduce aggressively` | Packed trim utility map with broad reuse; likely high-yield and lower-risk than the paired base color. |
| `T_Trim_Pipe_1_ORM` | `MI_Trim_Panel_7` | `Mask/ORM/Noise` | `reduce aggressively` | Pipe trim ORM is a secondary utility texture and fits the most aggressive class in this pass. |
| `T_Props_B_ORM` | `MI_Adv_Props_B` | `Mask/ORM/Noise` | `reduce aggressively` | Packed prop trim map should tolerate a larger cut than visible color textures. |
| `T_Props_A_ORM` | `MI_Adv_Props_A` | `Mask/ORM/Noise` | `reduce aggressively` | Same rationale as `T_Props_B_ORM`; utility-heavy and likely over-budget relative to its visual importance. |
| `T_Wall_Trim_2_N` | `MI_Basic_Trim_A`, `M_Wall_Trim_1_Adv` | `Background/Trim` | `reduce aggressively` | Secondary wall trim normal map is a strong VRAM target if the trim is not a hero close-up surface. |
| `T_Panel_2_B` | `MI_Trim_Panel_2` | `Background/Trim` | `reduce one tier` | Another repeated trim-sheet color map that should be stepped down ahead of hero materials. |
| `T_Panel_2_N` | `MI_Trim_Panel_2` | `Background/Trim` | `reduce aggressively` | Trim-sheet normal map with likely good tolerance for a stronger reduction than the paired base color. |
| `T_Panel_Trim7_N` | `MI_Trim_Panel_6` | `Background/Trim` | `reduce aggressively` | Reused trim normal in a modular panel family; strong candidate for early memory savings. |

### Map-Driven Inference Notes

- `MI_Master_Concrete_1` references `T_Concrete_5_B` and `T_Concrete_5_N`, which supports treating that set as active arena presentation content rather than dead folder residue.
- `MI_Fence_1` references `T_GridMat_N` and `T_GridMat_ORM`, which suggests the `GridMat` family is part of visible arena material usage and should not be aggressively reduced first.
- `MI_Trim_Panel_2`, `MI_Trim_Panel_3`, `MI_Trim_Panel_6`, `MI_Trim_Panel_7`, `MI_Basic_Trim_A`, `MI_Adv_Props_A`, and `MI_Adv_Props_B` reference the trim-sheet candidates, which supports treating them as repeated modular or background content for the first reduction batch.

## Obvious Risk And Candidate Notes

- `T_GridMat_*` looks like a shared arena surface set and is likely visible everywhere, so it should be classified carefully before reducing resolution.
- `T_Concrete_5_B`, `T_GridMat_N`, `T_GridMat_ORM`, `T_Concrete_5_N`, and `T_Concrete_2_N` are the top reported size targets and are likely to yield the best first-pass reduction if they are not hero-critical.
- `T_Concrete_*` and `T_Fence_*` have many normal and utility variants, which suggests they may be good reduction candidates if they are not hero-facing.
- `T_Metal_1_*`, `T_Trim_Metal_*`, and the `Trim_Textures/T_Panel_*` family look like repeated modular trim content. These are likely the best place to recover package size and VRAM without touching core combat visuals.
- `Props/T_Barrels_V1_Metal_*`, `Props/T_Boxes_2_*`, and `Props/T_FuelTank_1_*` are obvious prop sets with matching BC/N/ORM naming, so they should be checked for oversized normals and safe one-step downscales.
- `T_Panel_3_B`, `T_Panel_3_ORM`, `T_Trim_Pipe_1_ORM`, `T_Props_B_ORM`, `T_Props_A_ORM`, `T_Wall_Trim_2_N`, `T_Panel_2_B`, `T_Panel_2_N`, and `T_Panel_Trim7_N` are strong trim-sheet candidates because they were also reported among the largest assets.
- `T_SurfaceImperfections_1_M` and `T_Concrete_1_AO` are utility-style maps that may be strong candidates for lower resolution if they are not used as close-up hero surfaces.
- `T_Black_Metal_1` and `T_Concrete_1_R` are ambiguous from filename alone and need editor inspection before any aggressive reduction.

## Manual Verification Still Required In Editor

These items cannot be completed reliably from the CLI alone and must be checked in the Unreal Editor:

- texture streaming diagnostics on `PvPArena_map`
- per-texture imported resolution and `Max Texture Size`
- `Never Stream` usage
- whether any of the above assets are used as hero-facing materials versus background trim
- cooked/package-size comparison after the first batch of reductions
- visual screenshots for wall, trim, prop, and close-up hero surfaces

## Session Cautions

- Keep all edits confined to `/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction`. The main worktree at `/mnt/c/UE5CPP/PvPArena` already contains unrelated changes and should remain untouched.
- Treat the Task 2 table above as a working classification, not final truth. The absence of Unreal Editor GUI in this session means `Never Stream`, imported resolution, actual screen size, and texture streaming pressure still need manual confirmation.
- Do not treat packed map size alone as proof that an asset should be collapsed or deleted. `ORM` and grayscale textures are high-value reduction targets, but channel layout and material expectations must stay intact.
- `T_Concrete_2_N` is the main ambiguity in this batch. Confirm whether it is an actively sampled concrete normal or a redundant sibling before applying any destructive consolidation step.

## Task 3 Batch 1 Applied

The first low-risk reduction batch was applied on `2026-04-10` through `UnrealEditor-Cmd` in the `texture-budget-reduction` worktree.

### Applied Max Texture Size Caps

| Asset | Classification | Applied cap |
| --- | --- | --- |
| `T_Panel_3_ORM` | `Mask/ORM/Noise` | `512` |
| `T_Trim_Pipe_1_ORM` | `Mask/ORM/Noise` | `512` |
| `T_Props_B_ORM` | `Mask/ORM/Noise` | `512` |
| `T_Props_A_ORM` | `Mask/ORM/Noise` | `512` |
| `T_Wall_Trim_2_N` | `Background/Trim` | `1024` |
| `T_Panel_2_N` | `Background/Trim` | `1024` |
| `T_Panel_Trim7_N` | `Background/Trim` | `1024` |

### Execution Notes

- Unreal logged `max_texture_size 0 -> value` for all seven assets, which means these textures previously had no explicit per-asset cap and now do.
- The editor emitted `Unable to Check Out From Revision Control!` dialogs, but the save path still completed for every target and the `.uasset` files were rewritten in place.
- Headless startup loaded `PvPArena_map` during editor initialization, but this batch still does not replace manual visual validation on hero-facing angles.
- `T_Panel_3_B` and `T_Panel_2_B` were intentionally left unchanged in this batch to keep the first pass biased toward utility maps and trim normals.

## Task 3 Batch 2 Applied

The second reduction batch was applied on `2026-04-10` through `UnrealEditor-Cmd` after Batch 1 visual spot checks showed no major regression.

### Applied Max Texture Size Caps

| Asset | Classification | Applied cap |
| --- | --- | --- |
| `T_Panel_3_B` | `Background/Trim` | `1024` |
| `T_Panel_2_B` | `Background/Trim` | `1024` |
| `T_Concrete_5_N` | `Gameplay Visible` | `1024` |
| `T_Concrete_5_B` | `Gameplay Visible` | `1024` |
| `T_GridMat_ORM` | `Mask/ORM/Noise` | `1024` |

### Execution Notes

- Unreal logged `max_texture_size 0 -> 1024` for all five assets, so each of these textures now has an explicit per-asset cap.
- The same revision-control checkout warning appeared during save, but each package completed its save path and validation.
- This batch is intentionally more conservative than Batch 1 for the arena-defining surface set: `T_GridMat_ORM` was reduced one tier, while `T_GridMat_N` remains untouched.
- `T_Concrete_5_B` and `T_Concrete_5_N` now need the closest follow-up visual check because they are the first gameplay-visible concrete pair to receive an explicit cap.

## `T_Concrete_2_N` Investigation

`T_Concrete_2_N` remains the highest-risk unresolved concrete candidate in this pass, but the asset is not a dead or obviously unused texture.

### Current Asset Metadata

- Imported source: `D:/LOEEEBH/Concrete/T_Concrete_2_N.exr`
- Imported format: `TSF RGBA16F`
- Compression setting: `TC_HDR`
- Texture group: `TEXTUREGROUP_World`
- `Max Texture Size`: `0`
- `Never Stream`: `False`
- Size: `2048x2048`

### Reference Evidence

- `M_Master_Concrete_1.uasset` references `T_Concrete_2_N` directly alongside `T_Concrete_6_N`, `T_Concrete_Tiles_D`, `T_Concrete_Tiles_M`, and `T_Concrete_Wall_1_A_N`.
- I did not find a direct material-instance reference to `T_Concrete_2_N` in the current text scan, so the strongest evidence is that it is wired into the master concrete material rather than being an isolated leaf asset.
- The asset format is also unusual for a standard normal map: it is stored as HDR RGBA float data from an `.exr` source, which explains why it deserves separate handling from the normal-map batch.

### Disposition

- Disposition: `cap only`
- Status: `weakly referenced`

The current evidence supports keeping the asset in scope, but not treating it as unused or deleting it. It is active enough to remain a valid budget target, yet unusual enough that it should be handled separately from the normal Batch 3 trim reductions.

## Task 2 Batch 5 Candidate Set

Batch 5 is a one-asset follow-up that closes the remaining `cap only` concrete target without touching hero-facing grid materials.

### Selected Asset

| Asset | Classification | Planned cap | Reason |
| --- | --- | --- | --- |
| `T_Concrete_2_N` | `Gameplay Visible` | `1024` | The asset remains unusual because it is `TC_HDR` / `TSF RGBA16F`, but it is still only `2048x2048`; a one-tier cap is the narrowest safe way to reduce its budget without broader material changes. |

### Batch 5 Notes

- This batch intentionally changes only `Max Texture Size`; it does not alter compression, source format, or material wiring.
- The chosen `1024` cap matches the conservative one-tier precedent already used on other gameplay-visible concrete textures.
- Hero-facing sets such as `T_GridMat_N` still remain out of scope for this pass.

## Task 2 Batch 3 Candidate Set

Batch 3 stays narrow and deliberately avoids `T_Concrete_2_N`. The goal here is to keep the remaining pass low-risk while still recovering a meaningful amount of texture budget from utility and trim assets.

### Selected Assets

| Asset | Classification | Planned cap | Reason |
| --- | --- | --- | --- |
| `T_Metal_1_ORM` | `Mask/ORM/Noise` | `1024` | Shared ORM map used by visible trim and wall material paths, so a one-tier reduction is conservative but still worthwhile. |
| `T_Trim_Metal_2_ORM` | `Mask/ORM/Noise` | `512` | Packed trim utility map with no hero-facing role; it matches the aggressive utility precedent from Batch 1. |
| `T_Concrete_1_AO` | `Mask/ORM/Noise` | `512` | Standalone concrete AO utility texture that can tolerate a stronger reduction than a visible color or normal map. |
| `T_SurfaceImperfections_1_M` | `Mask/ORM/Noise` | `512` | Imperfection mask with low visual identity, making it a good low-risk utility target. |
| `T_Trim_Pipe_1_B` | `Background/Trim` | `1024` | Large trim-sheet base color at `4096x4096`; reducing it one tier is safer than touching the paired normal map first. |

### Batch 3 Notes

- `T_Concrete_2_N` is intentionally excluded because it remains tracked separately as `cap only`.
- `T_Trim_Pipe_1_N` is deferred for now because the current batch already covers enough utility reduction without taking on a 4096 normal map.
- The selected caps follow the earlier precedent: utility maps can drop to `512`, while conservative trim and support textures can step down to `1024`.

## Task 2 Batch 4 Candidate Set

Batch 4 is a one-asset follow-up that closes the remaining trim-pipe family gap left after Batch 3.

### Selected Asset

| Asset | Classification | Planned cap | Reason |
| --- | --- | --- | --- |
| `T_Trim_Pipe_1_N` | `Background/Trim` | `1024` | The paired ORM map is already capped to `512` and the base color is now capped to `1024`, so the remaining `4096x4096` trim normal is the next low-risk family cleanup target. |

### Batch 4 Notes

- This is intentionally narrower than Batch 3 because it touches a `4096` normal map rather than only utility maps.
- The chosen `1024` cap matches the earlier trim-normal precedent from Batch 1.
- `T_Concrete_2_N` remains outside this batch and still requires a separate follow-up.

## Task 3 Batch 3 Verified

Batch 3 was already present in the `texture-budget-reduction` worktree before this verification pass. A later verification run re-executed the same script through `UnrealEditor-Cmd` in the same worktree and confirmed the existing state.

### Applied Max Texture Size Caps

| Asset | Classification | Applied cap | Reason |
| --- | --- | --- | --- |
| `T_Metal_1_ORM` | `Mask/ORM/Noise` | `1024` | Repeated ORM map referenced by wall, fence, and trim material paths, but still tied to visible arena surfaces, so a one-tier cap is safer than an aggressive cut. |
| `T_Trim_Metal_2_ORM` | `Mask/ORM/Noise` | `512` | Trim ORM map on a modular material family and a good fit for the same aggressive utility reduction used in Batch 1. |
| `T_Concrete_1_AO` | `Mask/ORM/Noise` | `512` | Standalone AO utility texture in the wall-trim master material and a better package-size target than the paired visible textures. |
| `T_SurfaceImperfections_1_M` | `Mask/ORM/Noise` | `512` | Imperfection mask used as a supporting detail signal, making it a strong low-risk reduction target. |
| `T_Trim_Pipe_1_B` | `Background/Trim` | `1024` | The base-color counterpart to the already reduced `T_Trim_Pipe_1_ORM`, currently `4096x4096`, and still secondary enough to justify a one-tier cap. |

### Execution Notes

- A later verification pass re-ran `Scripts/texture_budget_batch_3.py` through `UnrealEditor-Cmd`, loaded `PvPArena_map`, exited `0`, and reported `texture_budget_batch_3 complete; changed 0 assets`.
- That verification pass read each target back as already set, so it confirmed the five Batch 3 asset edits were already present in the worktree rather than newly written by this turn.
- The same revision-control checkout warning appeared during save, but each package completed the save path and validation.
- Headless startup again loaded `PvPArena_map`, which keeps map context stable across batches, but this does not replace an editor-side visual pass.
- `T_Concrete_2_N` was intentionally excluded from Batch 3. Its `TC_HDR` and `TSF RGBA16F` metadata still make it a separate `cap only` follow-up instead of a low-risk trim-batch target.
- `T_Trim_Pipe_1_N` was also deferred behind `T_Trim_Pipe_1_B` because the family already took an aggressive ORM reduction and the base-color cut is the safer next step.

## Task 3 Batch 4 Applied

The fourth reduction batch was applied on `2026-04-10` through `UnrealEditor-Cmd` in the `texture-budget-reduction` worktree.

### Applied Max Texture Size Caps

| Asset | Classification | Applied cap | Reason |
| --- | --- | --- | --- |
| `T_Trim_Pipe_1_N` | `Background/Trim` | `1024` | The trim-pipe family already carried a `512` ORM cap and a `1024` base-color cap, making the remaining `4096x4096` normal map the next controlled one-tier reduction. |

### Execution Notes

- Unreal logged `max_texture_size 0 -> 1024` for `T_Trim_Pipe_1_N`, so the texture moved from an uncapped state to an explicit per-asset cap.
- The same revision-control checkout warning appeared during save, but the package completed its save path and validation.
- Headless startup again loaded `PvPArena_map`, which keeps the map context aligned with the earlier batches.
- This batch intentionally stops after the trim-pipe normal so the pass does not spill into hero-facing normals or the unusual `T_Concrete_2_N` asset.

## Task 3 Batch 5 Applied

The fifth reduction batch was applied on `2026-04-10` through `UnrealEditor-Cmd` in the `texture-budget-reduction` worktree.

### Applied Max Texture Size Caps

| Asset | Classification | Applied cap | Reason |
| --- | --- | --- | --- |
| `T_Concrete_2_N` | `Gameplay Visible` | `1024` | The asset remains unusual, but a `Max Texture Size` cap is still the narrowest low-risk way to trim it without changing its HDR format or master-material wiring. |

### Execution Notes

- Unreal logged `max_texture_size 0 -> 1024` for `T_Concrete_2_N`, so the texture moved from an uncapped state to an explicit per-asset cap.
- The same revision-control checkout warning appeared during save, but the package completed its save path and validation.
- Headless startup again loaded `PvPArena_map`, keeping the verification path aligned with the earlier batches.
- This batch intentionally did not touch `T_GridMat_N` or other hero-facing materials.

## Task 6 After-State Evidence

### Batch Summary

- Batch 1 changed `7` assets.
- Batch 2 changed `5` assets.
- Batch 3 changed `5` assets.
- Batch 4 changed `1` asset.
- Batch 5 changed `1` asset.
- Total changed assets in the current pass: `19`.

### Batch 3 On-Disk Size Deltas

These numbers compare the current worktree file sizes against the `HEAD` versions of the same assets:

| Asset | Before | After | Delta |
| --- | --- | --- | --- |
| `T_Metal_1_ORM` | `2650.91 KiB` | `2619.49 KiB` | `-31.42 KiB` |
| `T_Trim_Metal_2_ORM` | `1435.22 KiB` | `1413.82 KiB` | `-21.40 KiB` |
| `T_Concrete_1_AO` | `3266.36 KiB` | `3214.51 KiB` | `-51.85 KiB` |
| `T_SurfaceImperfections_1_M` | `2397.44 KiB` | `2339.98 KiB` | `-57.46 KiB` |
| `T_Trim_Pipe_1_B` | `7777.83 KiB` | `7743.17 KiB` | `-34.66 KiB` |
| **Total** | `17527.76 KiB` | `17331.00 KiB` | `-196.79 KiB` |

### Batch 4 On-Disk Size Delta

This number compares the current worktree file size against the `HEAD` version of the same asset:

| Asset | Before | After | Delta |
| --- | --- | --- | --- |
| `T_Trim_Pipe_1_N` | `8684.35 KiB` | `8665.07 KiB` | `-19.28 KiB` |

### Batch 5 On-Disk Size Delta

This number compares the current worktree file size against the `HEAD` version of the same asset:

| Asset | Before | After | Delta |
| --- | --- | --- | --- |
| `T_Concrete_2_N` | `16390.88 KiB` | `16247.80 KiB` | `-143.08 KiB` |

### Runtime And Visual Validation Notes

- User feedback after the earlier arena checks was that `PvPArena_map` did not show a large visible difference.
- Batch 3 completed under the same headless `PvPArena_map` startup path as Batch 1 and Batch 2.
- Batch 4 used the same headless startup path and only extended the already-touched trim-pipe family by capping the remaining normal map.
- Batch 5 used the same headless startup path and applied only a conservative `Max Texture Size` cap to `T_Concrete_2_N`.
- This environment still does not provide a trustworthy viewport-based visual validation pass or texture streaming stat capture, so final runtime verification remains partially open.

### Remaining Holdouts

- Hero-facing sets such as `T_GridMat_N`
  - intentionally untouched in this pass
  - still require manual confirmation before any aggressive reduction

## Commands Used

- `sed -n '1,120p' /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Config/DefaultScalability.ini`
- `sed -n '1,220p' /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Config/DefaultEngine.ini`
- `rg --files /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures/Trim_Textures`
- `find /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures -type f -name '*.uasset' | wc -l`
- `find /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures/Trim_Textures -type f -name '*.uasset' | wc -l`
- `find /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures -type f -name '*.uasset' -printf '%s\n' | awk '{sum+=$1} END {printf "%.2f MB\n", sum/1024/1024}'`
- `find /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena/Textures/Trim_Textures -type f -name '*.uasset' -printf '%s\n' | awk '{sum+=$1} END {printf "%.2f MB\n", sum/1024/1024}'`
- `rg --files /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena | rg 'PvPArena_map|Materials|Textures/Trim_Textures|Textures/Props|Textures/'`
- `rg -a -l 'T_Concrete_5_B|T_GridMat_N|T_GridMat_ORM|T_Concrete_5_N|T_Concrete_2_N|T_Panel_3_B|T_Panel_3_ORM|T_Trim_Pipe_1_ORM|T_Props_B_ORM|T_Props_A_ORM|T_Wall_Trim_2_N|T_Panel_2_B|T_Panel_2_N|T_Panel_Trim7_N' /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Content/PvPArena`
- `PYTHONPYCACHEPREFIX=/tmp/pythoncache python3 -m py_compile /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Scripts/texture_budget_batch_1.py`
- `PYTHONPYCACHEPREFIX=/tmp/pythoncache python3 -m py_compile /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Scripts/texture_budget_batch_2.py`
- `PYTHONPYCACHEPREFIX=/tmp/pythoncache python3 -m py_compile /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Scripts/texture_budget_batch_3.py`
- `PYTHONPYCACHEPREFIX=/tmp/pythoncache python3 -m py_compile /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Scripts/texture_budget_batch_4.py`
- `PYTHONPYCACHEPREFIX=/tmp/pythoncache python3 -m py_compile /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Scripts/texture_budget_batch_5.py`
- `powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat' PvPArenaEditor Win64 Development '-Project=C:\UE5CPP\PvPArena\.worktrees\texture-budget-reduction\PvPArena.uproject' -WaitMutex -NoHotReloadFromIDE"`
- `powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\.worktrees\texture-budget-reduction\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound -stdout -FullStdOutLogOutput -ExecutePythonScript='C:\UE5CPP\PvPArena\.worktrees\texture-budget-reduction\Scripts\texture_budget_batch_1.py'"`
- `powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\.worktrees\texture-budget-reduction\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound -stdout -FullStdOutLogOutput -ExecutePythonScript='C:\UE5CPP\PvPArena\.worktrees\texture-budget-reduction\Scripts\texture_budget_batch_2.py'"`
- `powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\.worktrees\texture-budget-reduction\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound -stdout -FullStdOutLogOutput -ExecutePythonScript='C:\UE5CPP\PvPArena\.worktrees\texture-budget-reduction\Scripts\texture_budget_batch_4.py'"`
- `powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\.worktrees\texture-budget-reduction\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound -stdout -FullStdOutLogOutput -ExecutePythonScript='C:\UE5CPP\PvPArena\.worktrees\texture-budget-reduction\Scripts\texture_budget_batch_5.py'"`
- `git -C /mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction status --short --untracked-files=all`
