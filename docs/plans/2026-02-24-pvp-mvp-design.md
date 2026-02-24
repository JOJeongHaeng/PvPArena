# PvPArena Standard MVP Design

**Date:** 2026-02-24  
**Template/Base:** Unreal Engine 5 ThirdPerson  
**Project Path:** `C:\UE5CPP\PvPArena`

## 1. Scope

Target is a **standard PvP MVP** with:
- 1v1 match
- Two attack types (melee + ranged)
- Score/round rules
- Basic UI (HP, score, timer, status)
- Listen Server hosting model
- Mixed implementation: core gameplay in C++, tuning/presentation in Blueprint

Out of scope for this MVP:
- Matchmaking/backend services
- Ranked systems
- Team modes beyond 1v1
- Advanced anti-cheat

## 2. Architecture

### Authority model
- Server-authoritative gameplay (Listen Server)
- Client sends action requests via RPC
- Server validates and applies combat/game-state changes

### Class responsibilities
- `APvPArenaGameMode` (Server only)
  - Round flow
  - Win condition checks
  - Respawn orchestration
- `APvPArenaGameState`
  - Replicated timer/state/scoreboard-level match data
- `APvPArenaPlayerState`
  - Replicated per-player stats (kills/deaths/score)
- `APvPArenaCharacter`
  - Health, alive/dead state, damage handling, death flow hooks
  - Input entry points mapped to server requests
- `UPvPCombatComponent` (or equivalent)
  - Melee/ranged attack execution
  - Cooldown/validation logic
- Blueprint layer
  - Animation montages, VFX/SFX, UMG widgets and binding

## 3. Combat & Network Data Flow

1. Client inputs attack.
2. Client calls `Server` RPC (reliable/unreliable based on action criticality).
3. Server validates:
   - attacker alive
   - cooldown ready
   - range/line-of-sight as applicable
4. Server resolves hit:
   - Melee: server trace/overlap
   - Ranged (MVP): server-side hitscan
5. Server applies damage and updates replicated health/state.
6. `OnRep_Health` and state updates drive UI + feedback on clients.
7. On death:
   - Server updates killer/victim `PlayerState`
   - Server starts respawn timer
   - Server respawns at selected spawn point

## 4. Round & Win Rules

Default config for MVP:
- Score limit: `5` kills
- Round time: `180` seconds
- Respawn delay: `3` seconds

Resolution:
- First to score limit wins immediately.
- If timer expires, higher score wins.
- Tie at timeout enters sudden death (next kill wins).

Round transition:
- Show short result phase (`~5s`)
- Reset positions/round state
- Start next round without reloading map

## 5. UI/HUD Scope

Minimum HUD elements:
- Local HP bar + value
- Match timer and round status (`Playing`, `RoundEnd`, `SuddenDeath`)
- Player scores (1v1)
- Kill/death and round result notifications
- Respawn countdown when dead

Principles:
- Data source from replicated C++ state
- UI presentation in Blueprint/UMG
- Event-driven updates preferred over Tick-heavy bindings

## 6. Validation Criteria

Done when all are true in PIE 2-player (Listen Server + Client):
- Movement/jump/melee/ranged all function for both players
- Damage/death/respawn are synchronized and consistent
- Score/timer/state UI updates correctly on both peers
- Win conditions trigger correctly (score limit, timeout, sudden death)
- Round reset restores expected state (positions, input, health)

## 7. Risks & Mitigations

- Risk: client-side desync in combat feedback
  - Mitigation: server-authoritative damage and clear replication events
- Risk: scope creep before playable loop
  - Mitigation: lock MVP to 1v1 + 2 attack types + simple HUD
- Risk: brittle BP/C++ boundary
  - Mitigation: keep gameplay authority in C++, Blueprint for visuals/tuning only
