# Ranged Projectile Visuals Design

**Goal:** Make the ranged projectile visibly read as a medium-sized magic orb with an energy shell, and add a visible impact effect on collision.

**Context**

- The ranged projectile currently has gameplay-only collision and movement plus an optional debug line.
- Crosshair alignment and release-time aim snapshot are already fixed on `main`, so visuals should follow the existing collision sphere exactly.
- The content folder already contains reusable Niagara systems and VFX pack materials, so the first pass should favor wiring default assets over introducing new gameplay behavior.

**Chosen Approach**

- Add a `UStaticMeshComponent` for the orb core and attach it directly to the projectile collision root at zero offset.
- Add a `UNiagaraComponent` for the outer energy shell, also centered on the collision sphere.
- Add a configurable `UNiagaraSystem*` impact effect and spawn it once at the hit location just before the projectile is destroyed.

**Why This Approach**

- Separating the mesh core from the Niagara shell keeps the projectile center visually honest and easy to tune against collision.
- Reusing existing pack assets keeps the change small and shippable without requiring new art content.
- Spawning the impact effect independently avoids coupling hit feedback to the travel effect.

**Data Flow**

1. Projectile is constructed with collision, movement, visual mesh, and Niagara shell components.
2. Default assets are loaded in the constructor and attached with zero relative offset so the visuals sit on the collision sphere.
3. The combat component spawns and initializes the projectile exactly as before.
4. On hit, the projectile applies damage, spawns the impact effect at the hit location, then destroys itself.

**Constraints**

- Visual components must not change projectile collision, launch direction, or current ranged timing behavior.
- Defaults should stay editable from C++ defaults and Blueprints for later tuning.
- The first pass should use existing assets already available in the repository.

**Testing**

- Update ranged projectile defaults coverage to assert the projectile now creates a mesh component and Niagara shell component.
- Assert the impact effect default is assigned.
- Re-run ranged projectile defaults after build.
