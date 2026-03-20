# Best-Of-Five Match Flow Design

**Goal:** Add a lobby flow, best-of-five round-win match rule, and a final results screen so the first player to win 3 rounds becomes the match winner.

**Recommended Approach:** Extend the existing in-map `GameMode`/`GameState`/HUD flow instead of introducing level travel. Add a replicated match-phase state, replicated round-win counts per player, and two lightweight HUD overlays for `Lobby` and `MatchEnd`.

## State Model

- Keep round combat state in `EPvPARoundState`.
- Add a higher-level match phase in `GameState`: `Lobby`, `Playing`, `MatchEnd`.
- `Lobby` waits until at least two player controllers exist, then starts a fresh match.
- `Playing` preserves current round timer, sudden death, respawn, and round reset behavior.
- `MatchEnd` replaces the next-round loop once a player reaches 3 round wins.

## Scoring Model

- Keep `RoundKills/RoundDeaths` and `MatchKills/MatchDeaths` as-is.
- Add `RoundWins` to `PlayerState`.
- A round winner is determined by the current existing round-end logic:
  - reaching score limit
  - sudden death kill
  - timeout with non-tied round kills
- The round winner gains `RoundWins += 1`.
- If `RoundWins >= 3`, the match moves to `MatchEnd` instead of resetting into the next round.

## UI Model

- Keep the current status panel for live combat information.
- Add a full-screen centered `LobbyPanel` with text such as `Waiting for players...` and `Match starts when 2 players join`.
- Add a full-screen centered `MatchResultPanel` with winner/loser text, round-win scoreline, and a countdown back to lobby.
- Hide combat-only announcement text while in `Lobby` or `MatchEnd`.

## Reset Rules

- `HandleRoundReset()` continues to reset only round-local stats.
- A new `ResetMatchState()` path clears:
  - `RoundWins`
  - `RoundKills/RoundDeaths`
  - `MatchKills/MatchDeaths`
  - timers and winner flags
- After `MatchEnd`, wait a short delay, then return to `Lobby` and wait for enough players to start the next match.

## Testing

- Extend round win-condition tests to cover 3-round-win match completion.
- Extend player-state score tests to cover `RoundWins`.
- Add game-state defaults/replication coverage for the new match phase and target round-wins value.
- Keep HUD verification lightweight unless UI wiring needs explicit regression coverage.
