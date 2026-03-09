# Next Session Handoff

## Project
- Path: `C:\UE5CPP\PvPArena`
- Branch: `main`

## Latest Commits (most relevant)
- `9f78978` feat: split round and match stats with improved hud readability
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
- Player stats split completed:
  - `Round K/D` resets per round
  - `Match K/D` persists across round reset

## Important Notes
- Text HUD path (`APvPArenaHUD`) is the active stable path.
- Build can fail at link stage if `UnrealEditor.exe` is open and locks `UnrealEditor-PvPArena.dll`.
- Related implementation/progress detail: `docs/plans/2026-03-07-session-progress.md`.

## Next Task
1. UMG migration (incremental)
- Mirror current text HUD data in UMG first (no gameplay logic move), then switch display path.

2. Match-level win policy
- Decide if match should end on `MatchKills` target (best-of / first-to-N) in addition to per-round win.

## Suggested Start Prompt for Next Codex Session
"이전 세션 이어서 진행.
프로젝트: C:\UE5CPP\PvPArena
브랜치: main
handoff 문서: docs/plans/next-session.md
다음 작업: HUD 가독성 개선 + 라운드 통계 정책 확정(라운드/매치 분리 여부)"
