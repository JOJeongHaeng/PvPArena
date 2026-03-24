# Attack Audio Design

**Goal:** Add attack-only audio so melee attacks play a short attached loop-like fire sound and ranged hit notifies play an attached explosion sound.

**Recommended Approach:** Keep audio on `APvPArenaCharacter` and attach playback to the player character rather than world impact points. Reuse the existing melee montage path and ranged hit-notify path so the sound timing stays aligned with current combat animation flow.

## Audio Model

- Left click melee attack plays `/Game/PvPArena/Audio/Fire01_Cue` from the attacker character.
- Melee audio starts with the melee montage/effect path and stops after `1.5` seconds.
- Right click ranged attack plays `/Game/PvPArena/Audio/Explosion_Cue` when the ranged attack hit notify resolves.
- Ranged audio is an attached one-shot on the attacker character.
- No victim hit audio is added.

## Networking

- Melee audio can piggyback on the existing melee montage playback on each client.
- Ranged audio needs a multicast hook because the ranged notify currently resolves on the authority path and remote simulated clients do not trigger the notify locally.

## Testing

- Verify the character exposes melee/ranged attack audio properties.
- Verify the default assets point at the project-owned `Fire01_Cue` and `Explosion_Cue`.
- Verify the melee attack audio duration defaults to `1.5`.
- Verify the ranged attack audio multicast RPC exists.
