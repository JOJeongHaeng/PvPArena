# Attack Audio Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add player-attached melee and ranged attack sounds without adding hit or victim audio.

**Architecture:** Extend `APvPArenaCharacter` with attack audio assets and attached audio components. Start melee audio from the melee montage path with a fixed stop timer, and multicast ranged audio from the server-authoritative hit-notify path so all clients hear the attacker sound.

**Tech Stack:** Unreal Engine 5 C++, `UAudioComponent`, `USoundCue`/`USoundBase`, replication RPCs, Unreal automation tests.

---

### Task 1: Add failing defaults coverage for attack audio

**Files:**
- Create: `/mnt/c/UE5CPP/PvPArena/.worktrees/feature-hit-feedback/Source/PvPArena/Private/Tests/CharacterAttackAudioDefaultsTest.cpp`

**Step 1: Write the failing test**

- Assert melee attack audio, ranged attack audio, melee audio duration, and ranged audio multicast RPC exist.
- Assert defaults point to `/Game/PvPArena/Audio/Fire01_Cue` and `/Game/PvPArena/Audio/Explosion_Cue`.
- Assert melee duration defaults to `1.5`.

**Step 2: Run test to verify it fails**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat' PvPArenaEditor Win64 Development '-Project=C:\UE5CPP\PvPArena\.worktrees\feature-hit-feedback\PvPArena.uproject' -WaitMutex -NoHotReloadFromIDE'"`

Expected: FAIL on missing audio properties/RPC during compile.

**Step 3: Write minimal implementation**

- Add attack audio assets, attached components, stop timer, and ranged multicast playback.

**Step 4: Run test to verify it passes**

Run the build plus targeted automation and expect PASS.

### Task 2: Wire melee and ranged playback into existing attack flow

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/.worktrees/feature-hit-feedback/Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/.worktrees/feature-hit-feedback/Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`

**Step 1: Keep the test failing if needed**

- Extend the defaults test only if a public property or RPC needs explicit coverage.

**Step 2: Run verification to confirm red**

- Re-run build/automation if the failing surface changed.

**Step 3: Write minimal implementation**

- Start `Fire01_Cue` from melee montage playback and stop it after `1.5` seconds.
- Multicast `Explosion_Cue` playback from the ranged notify hit path.

**Step 4: Run verification**

- Confirm the targeted automation passes and the project builds.

### Task 3: Verify attack audio change end-to-end

**Files:**
- No code changes unless defects are found.

**Step 1: Run build**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat' PvPArenaEditor Win64 Development '-Project=C:\UE5CPP\PvPArena\.worktrees\feature-hit-feedback\PvPArena.uproject' -WaitMutex -NoHotReloadFromIDE'"`

Expected: build succeeds.

**Step 2: Run targeted automation**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\.worktrees\feature-hit-feedback\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Character.AttackAudioDefaults+PvPArena.Character.MeleeEffectsDefaults+PvPArena.Character.DeathFlow; Quit'"`

Expected: PASS.

**Step 3: Manual verification**

- Left click once and confirm `Fire01_Cue` follows the attacker for about `1.5` seconds.
- Right click and confirm `Explosion_Cue` plays on the attacker at the hit notify timing.
- Confirm no victim-hit audio was added.
