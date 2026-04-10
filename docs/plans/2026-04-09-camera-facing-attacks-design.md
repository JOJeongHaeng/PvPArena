# Camera-Facing Attacks Design

**Goal:** Make both left-click and right-click attacks continuously rotate the character toward the camera yaw while preserving movement input direction, and increase left-click attack speed.

**Current Context**

- Attack-time movement suppression is already removed from `APvPArenaCharacter`.
- Ranged attack still owns the only attack-facing update path through `UpdateRangedAttackFacing`.
- Melee attack currently does not rotate the character toward the camera during the attack.
- Melee attack playback speed is driven by `MeleeAttackPlayRate`, currently `2.0f`.

## Recommended Approach

Use a single attack-facing rule for both melee and ranged attacks: when any attack is active, rotate the actor toward `Controller->GetControlRotation().Yaw` every tick using the existing interpolation speed. Keep movement input calculation unchanged so strafing and camera-relative movement continue to work as before.

This keeps combat timing and hit-notify flow intact while making attack presentation consistent across left-click and right-click.

## Gameplay Rules

- While melee attack is active, the character rotates toward camera yaw every tick.
- While ranged charge/release is active, the character rotates toward camera yaw every tick.
- Movement input direction remains unchanged by the attack-facing update.
- Melee front-hit traces continue using actor forward direction, which now tracks the camera-facing body orientation.

## Animation Rules

- This change does not require a new AnimBP rule by itself.
- If the user keeps the upper-body montage setup, attack-facing rotation should now visually align the torso and root direction with the camera.

## Code Changes

- Generalize attack-facing update so it runs for melee and ranged attacks.
- Resolve the target attack yaw from `Controller->GetControlRotation().Yaw`.
- Keep `RangedAttackTargetYaw` updated for compatibility with existing ranged code and tests.
- Raise `MeleeAttackPlayRate` to speed up left-click attack playback.

## Testing Strategy

- Add melee automation coverage that verifies attack-facing turns the actor toward controller yaw during melee.
- Update ranged automation coverage to verify attack-facing tracks controller yaw during ranged attack even without a ranged-specific facing lock.
- Add a melee default-value assertion for the higher `MeleeAttackPlayRate`.
- Rebuild and run focused melee/ranged automation tests.
