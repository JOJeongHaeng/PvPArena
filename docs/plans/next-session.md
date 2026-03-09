# Next Session Handoff

## Project
- Path: `C:\UE5CPP\PvPArena`
- Branch: `main`

## Latest Commits (most relevant)
- `fd5830f` feat: add round-end countdown, auto-reset, and hud result text
- `1e82b24` chore: save config and content updates for pvp map and input assets
- `8c19472` feat: add respawn invulnerability window with damage gating
- `ebba202` fix: restore stable legacy hud rendering path
- `e0123f7` fix: create HUD via local player controller for reliable viewport display

## Current Status
- Core PvP loop works in PIE 2-player Listen Server:
  - movement/jump
  - melee/ranged server-authoritative hit detection
  - health/death/respawn
  - score and round state updates
- Respawn invulnerability implemented (1.5s)
- Round flow implemented:
  - `RoundEnd` result phase (5s countdown)
  - automatic next-round reset (without map reload)
  - sudden death finalization (next valid kill ends round)
- Text HUD (`APvPArenaHUD`) now displays:
  - `Round` state
  - `Result: Win/Lose/Draw` during round end
  - `Next Round In: N` countdown

## Important Notes
- Text HUD path (`APvPArenaHUD`) is the active stable path.
- Build can fail at link stage if `UnrealEditor.exe` is open and locks `UnrealEditor-PvPArena.dll`.
- Related implementation/progress detail: `docs/plans/2026-03-07-session-progress.md`.

## Next Task
1. HUD polish
- Improve readability/layout/colors for round-end result and countdown text.

2. Round stats policy
- Decide whether `Kills/Deaths` should reset each round or persist for whole match.
- If persistent match score is required, split to `RoundKills` vs `MatchKills`.

3. UMG migration (incremental)
- Mirror current text HUD data in UMG first (no gameplay logic move), then switch display path.

## Suggested Start Prompt for Next Codex Session
"이전 세션 이어서 진행.
프로젝트: C:\UE5CPP\PvPArena
브랜치: main
handoff 문서: docs/plans/next-session.md
다음 작업: HUD 가독성 개선 + 라운드 통계 정책 확정(라운드/매치 분리 여부)"
