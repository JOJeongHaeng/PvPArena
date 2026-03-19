# Ranged Charge Hold Design

**Goal:** Replace the current right-click instant ranged attack with a hold-to-charge flow that plays a start section, loops a hold section while the button is held, plays either release or cancel on button release, and only fires the projectile when release succeeds after a 1.0 second minimum hold.

**Current Context**

- `APvPArenaCharacter` currently treats ranged attack as a single begin/notify/finish sequence driven by `MTG_RangedAttack_RightClick`.
- `UPvPCombatComponent` consumes ranged cooldown at attack start and spawns a projectile on notify.
- Character facing already locks to the ranged target yaw during ranged attack.
- The user has split `MTG_RangedAttack_RightClick` into `Start`, `Hold`, `Release`, and `Cancel`, with `Hold` looping.

## Recommended Approach

Use the existing montage-driven architecture and promote ranged attack into a small replicated state machine:

- `Pressed` starts ranged charge if the character is eligible.
- `Start` transitions into looping `Hold`.
- `Released` before 1.0 seconds jumps to `Cancel` and never consumes cooldown.
- `Released` after 1.0 seconds jumps to `Release`, consumes cooldown, and only fires at the `Release` notify.
- During hold, the active camera setup is not replaced; instead the existing camera owner will receive a smooth rightward aim offset signal.

This keeps the current notify-driven combat model intact and avoids introducing an AnimBP-only ranged system.

## State Model

Add an explicit ranged charge lifecycle to `APvPArenaCharacter`:

- `Idle`
- `ChargingStart`
- `ChargingHold`
- `ReleasePending`
- `CancelPending`

State transitions:

- `Pressed` while idle and eligible -> `ChargingStart`
- montage reaches `Hold` -> `ChargingHold`
- `Released` during start/hold and held long enough -> `ReleasePending`
- `Released` during start/hold and held too briefly -> `CancelPending`
- `Release` montage end -> `Idle`
- `Cancel` montage end -> `Idle`

Supporting data:

- hold start time
- whether input is currently held
- whether release has been committed
- whether cooldown has been consumed
- camera aim offset alpha

## Input And Cooldown Rules

Input changes:

- Right-click must bind both press and release events.
- Press no longer immediately commits a ranged attack.
- Release decides whether the attack cancels or commits.

Cooldown changes:

- Cooldown is consumed only when the release path is committed.
- Cancels do not start cooldown.
- The HUD continues to read remaining cooldown from replicated combat state.

## Montage Rules

`MTG_RangedAttack_RightClick` is treated as:

- `Start`: charge-in animation
- `Hold`: looping charge pose
- `Release`: attack execution
- `Cancel`: charge abort return

Rules:

- `Hold` loops to itself.
- `PvPAnimNotify_RangedHit` must exist only in `Release`.
- Code controls `SetNextSection` / `JumpToSection` to leave `Hold`.
- Montage end delegate is still the single cleanup point.

## Camera Behavior

Do not replace the current camera setup.

Instead:

- detect the currently active spring arm or camera owner already used by the player
- cache its original lateral offset at charge start
- while charging, interpolate toward a configurable rightward offset
- on release or cancel, interpolate back to the cached default

This preserves the user’s current camera feel while still giving an over-the-shoulder aiming shift.

## Failure And Cancel Behavior

Cancel conditions:

- released before 1.0 seconds
- montage cannot play
- character dies
- another mutually exclusive attack blocks start

Cancel guarantees:

- no projectile spawned
- no cooldown consumed
- movement restored
- facing lock cleared
- camera offset restored

## Testing Strategy

Add or update automation coverage for:

- ranged charge state starts on press and does not fire immediately
- releasing before 1.0 seconds cancels and does not consume cooldown
- releasing after 1.0 seconds commits release and consumes cooldown
- release notify still fires exactly once
- ranged state cleanup restores movement/facing
- camera offset state variables move toward aim offset and return to default

Asset validation remains editor-side:

- `Hold` loops cleanly
- `Release` contains the ranged notify
- `Cancel` contains no ranged notify
