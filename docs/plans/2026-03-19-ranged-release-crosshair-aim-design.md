# Ranged Release Crosshair Aim Design

**Goal:** Remove the ranged projectile Niagara effect and make charged ranged release fire toward the world point under the HUD crosshair, while the character turns smoothly toward that same aim point during hold.

**Context**

- The current charged ranged flow already has hold/release/cancel state and a HUD crosshair.
- The projectile currently carries a Niagara effect and a fixed relative offset.
- Character facing currently locks to a yaw captured at charge start instead of continuously following the player aim during hold.

**Chosen Approach**

- During local ranged hold, compute a world aim point from the camera through the crosshair with a visibility trace every tick.
- Convert that aim point into a target yaw and replicate that yaw to the server through the existing release/start flow so the character keeps rotating naturally during hold and the release uses the latest aim.
- Remove the projectile Niagara component entirely so the spawned projectile is gameplay-only.

**Why This Approach**

- It matches the player-visible crosshair rather than a stale hold-start direction.
- The per-tick work is limited to the locally controlled character while input is held, so cost stays small.
- It avoids introducing a new targeting system or animation-only workaround.

**Data Flow**

1. Player holds right-click.
2. Local character traces from the camera through the crosshair each tick while hold is active.
3. The character updates `RangedAttackTargetYaw` from the hit point and keeps using interpolation for smooth turning.
4. On release, the latest crosshair-derived aim is used again to determine projectile direction.
5. Combat component spawns the projectile toward the resolved world hit point or fallback end point.

**Constraints**

- Aim tracing should run only for the locally controlled character while ranged charge input is held.
- The server should still own projectile spawning and damage.
- Existing ranged timing, cooldown, and HUD behavior should remain intact.

**Testing**

- Extend projectile defaults coverage to assert the projectile no longer creates a Niagara component.
- Extend ranged timing coverage to assert hold updates the target yaw from current control rotation rather than only charge-start state.
- Re-run ranged projectile defaults and ranged timing automation after build.
