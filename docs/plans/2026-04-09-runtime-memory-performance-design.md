# Runtime Memory And Performance Design

**Goal:** Reduce packaged-build memory pressure and improve runtime responsiveness without introducing first-use combat hitches or changing core combat behavior.

**Current Context**

- `APvPProjectile` enables ticking for every projectile instance, but `Tick()` only draws debug lines when `bDrawProjectileDebug` is enabled.
- `UPvPArenaHUDWidget` already refreshes HUD state on a `0.15f` repeating timer, but `NativeTick()` still runs every frame only to call `RefreshCrosshairVisibility()`.
- `UPvPArenaOverheadStatusWidget` refreshes display state every frame through `NativeTick()`, including a visibility trace for crosshair targeting.
- HUD background music assets are hard-loaded in the widget constructor. They are not combat-critical, but changing their loading model is optional and lower priority than removing wasteful per-frame work.

## Recommended Approach

Start with low-risk runtime optimizations that remove unnecessary per-frame work and reduce the number of always-active ticking objects. Keep combat-critical assets and behavior unchanged for now. This targets the user's "game feels heavy" complaint without risking first-use attack delays.

## Optimization Rules

- Disable projectile ticking by default and only enable it when projectile debug drawing is explicitly active.
- Remove HUD per-frame refresh when the same work already happens on the existing refresh timer.
- Replace overhead widget per-frame refresh with a lower-frequency timer so targeting highlight and health/nameplate updates remain responsive without tracing every frame for every widget.
- Leave combat montage, projectile combat effects, and attack assets as hard references in this pass.
- Keep HUD BGM asset loading unchanged unless the low-risk runtime changes are insufficient.

## Code Changes

- `APvPProjectile`
  - Set `PrimaryActorTick.bCanEverTick` to `false` by default.
  - Enable actor ticking only when projectile debug drawing is active.
  - Reset debug draw state when ticking is disabled.
- `UPvPArenaHUDWidget`
  - Remove `NativeTick` override and rely on the existing timer-driven `RefreshWidgetData()` path for crosshair visibility updates.
- `UPvPArenaOverheadStatusWidget`
  - Replace `NativeTick` with a repeating timer started in `NativeConstruct` and cleared in `NativeDestruct`.
  - Refresh at a modest cadence that preserves highlight responsiveness while avoiding per-frame crosshair trace work for all overhead widgets.

## Testing Strategy

- Add projectile coverage that verifies debug draw ticking is disabled by default.
- Add HUD widget coverage that verifies crosshair visibility continues to refresh through the existing polling path without requiring `NativeTick`.
- Add overhead widget coverage that verifies refresh can be triggered without frame tick and that timer cleanup happens safely on destruction.
- Rebuild the project and run focused automation covering projectile and HUD/UI behavior.
