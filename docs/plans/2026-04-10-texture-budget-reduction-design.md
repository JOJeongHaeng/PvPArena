# Texture Budget Reduction Design

**Goal:** Reduce both packaged build size and runtime texture memory usage for `PvPArena` without a broad visual regression pass or material-system rewrite.

**Current Context**

- Project-level texture policy is minimal. [`Config/DefaultEngine.ini`](/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Config/DefaultEngine.ini) and [`Config/DefaultScalability.ini`](/mnt/c/UE5CPP/PvPArena/.worktrees/texture-budget-reduction/Config/DefaultScalability.ini) only define a `1000MB` streaming pool and do not establish asset-class texture budgets.
- The main reduction candidates are concentrated in `Content/PvPArena/Textures` and `Content/PvPArena/Textures/Trim_Textures`, where hard-surface base color, normal, ORM, trim, and utility textures are clustered.
- Character textures and broad starter/mannequin content exist, but they are higher-risk for noticeable visual regression and are not the best first pass for a dual package-size and VRAM target.

## Recommended Approach

Use a texture audit plus targeted asset normalization pass. Keep hero-facing surfaces stable, aggressively reduce secondary trim and utility textures, and remove avoidable streaming exceptions. This avoids a heavy material rewrite while still attacking both cooked asset size and live texture residency.

## Scope

- First pass:
  - `Content/PvPArena/Textures`
  - `Content/PvPArena/Textures/Trim_Textures`
- Deferred unless the first pass is insufficient:
  - `Content/Characters/.../Textures`
  - UI textures
  - Marketplace or starter-pack content outside the active arena presentation path

## Asset Classification Rules

- `Hero`
  - Large, repeatedly inspected surfaces that define the arena's look in common camera positions
  - Preserve current resolution unless testing shows obvious headroom
- `Gameplay Visible`
  - Frequently seen during play, but not close-up hero assets
  - Candidates for one-step resolution reduction
- `Background/Trim`
  - Repeating surfaces, modular trims, secondary panels, fence details, and filler materials
  - Primary reduction targets
- `Mask/ORM/Noise`
  - Utility textures, grayscale masks, packed channels, imperfections, and auxiliary maps
  - Most aggressive reduction targets because they often tolerate lower resolution better than base color

## Optimization Rules

- Reduce non-hero `Normal`, `ORM`, and grayscale utility textures before touching hero base color.
- Prefer `Max Texture Size` or imported resolution caps over ad hoc material graph workarounds.
- Remove or minimize `Never Stream` usage except where correctness clearly requires it.
- Preserve packed `ORM` textures and look for opportunities to replace standalone grayscale maps with existing packed data where practical.
- Avoid changing material graphs in this pass unless an optimization cannot be realized through texture asset settings alone.
- Do not lower texture quality globally through scalability defaults as a substitute for asset cleanup. The first pass should improve the base content budget itself.

## Project Configuration Changes

- Add texture LOD policy entries in project config only if they support the asset audit decisions and remain easy to reason about.
- Keep `r.Streaming.PoolSize=1000` as a measurement reference during this pass rather than masking bad asset budgets by increasing the pool.
- If additional per-group LOD settings are introduced, keep them limited to clearly named groups used by the audited assets.

## Validation Strategy

- Use `PvPArena_map` as the baseline validation map.
- Validate packaged size through asset-level size comparison and cooked/package outputs when available.
- Validate runtime memory through:
  - texture streaming pool warnings
  - in-editor or packaged texture memory stats on the baseline map
  - spot checks for visible mip drops or obvious normal-map degradation
- Compare before/after on the same hardware and map state.

## Risks And Mitigations

- Risk: visible softness on trims and repeated wall panels
  - Mitigation: classify assets first and reduce background/secondary assets before hero assets
- Risk: incorrect roughness/metal response after repacking or compression changes
  - Mitigation: avoid channel-layout changes unless verified against the consuming material
- Risk: hidden dependencies on non-streaming textures
  - Mitigation: remove streaming exceptions selectively and validate the arena map after each batch

## Testing Strategy

- Record a before/after asset spreadsheet or checklist for targeted textures, including resolution, group, max size, and streaming exceptions.
- Run map validation in `PvPArena_map` after each asset batch.
- Rebuild or recook the project as needed to confirm packaged size reduction from the audited assets.
- Capture texture streaming diagnostics before and after the pass to confirm the runtime budget actually improved.

## Approved Next Phase

The next execution phase should prioritize ambiguity removal before broader reduction:

1. Investigate `T_Concrete_2_N` first.
2. Use that result to define a small `Batch 3`.
3. Fold the new evidence into the baseline document as the after-state record.

### Why This Order

- `T_Concrete_2_N` is currently the highest-risk unknown in the candidate set.
- CLI inspection shows weak direct reference evidence compared with the confirmed Batch 1 and Batch 2 assets.
- The asset metadata also stands out from the rest of the concrete normal set because it appears to use `TC_HDR` and `TSF RGBA16F` rather than the expected normal-map settings.
- Closing that ambiguity first reduces the chance of spending Batch 3 on the wrong target or missing an outsized savings opportunity.

### Phase 1: `T_Concrete_2_N` Investigation

- Confirm whether the asset is actively consumed by arena materials or effectively dormant.
- Record its current imported format, compression, LOD group, `Max Texture Size`, and any other settings that explain its current budget cost.
- Produce a single disposition for the asset:
  - `keep`
  - `cap only`
  - `format-fix candidate`
  - `unused or deferred`

### Phase 2: `Batch 3` Design

Only after the `T_Concrete_2_N` outcome is recorded should the pass define a small additional batch.

Preferred `Batch 3` profile:

- 3 to 5 assets maximum
- utility, ORM, AO, imperfection, or trim textures first
- avoid broad hero-surface reductions
- keep the batch isolated enough that visual fallout can be attributed quickly

Current likely candidates if investigation does not elevate a different asset:

- `T_Metal_1_ORM`
- `T_Trim_Metal_2_ORM`
- `T_Concrete_1_AO`
- `T_SurfaceImperfections_1_M`
- one of `T_Trim_Pipe_1_N` or `T_Trim_Pipe_1_B`

### Phase 3: After-State Evidence

Once Phase 1 and Phase 2 are complete, update the baseline notes with:

- the full Batch 1, Batch 2, and Batch 3 change list
- before and after size deltas for changed assets
- runtime validation notes from `PvPArena_map`
- holdouts that were intentionally left unchanged
- the final `T_Concrete_2_N` rationale
