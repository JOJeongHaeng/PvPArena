# Free-For-All Single Match Design

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Convert free-for-all into a single 3-minute match with first-to-5 early victory and leader-only sudden death, while keeping team-versus as a 3-round mode.

**Architecture:** `APvPArenaGameMode` owns the mode split. Team-versus keeps the round loop and team score aggregation. Free-for-all uses one continuous playing phase, ends immediately on 5 kills, and on timeout either ends the match with a unique leader or enters `SuddenDeath` for tied leaders only while non-leaders spectate.

**Tech Stack:** Unreal Engine 5.5, replicated `GameMode`/`GameState`/`PlayerState`, automation tests.

---

### Task 1: Lock the new rules in tests

**Files:**
- Modify: `Source/PvPArena/Private/Tests/GameplayTempoDefaultsTest.cpp`
- Modify: `Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp`
- Modify: `Source/PvPArena/Private/Tests/EliminationRespawnBoundaryTest.cpp`
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetTeamScoreTest.cpp`

Add expectations for:
- free-for-all score limit `5`
- free-for-all duration `180`
- team-versus wins-to-win `3`
- no `RoundScoreText`
- free-for-all sudden death keeps tied leaders only

### Task 2: Make match settings mode-aware

**Files:**
- Modify: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Modify: `Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`

Add helpers that resolve:
- current score limit by mode
- current round/match duration by mode
- whether free-for-all is in leader-only sudden death
- team round-win aggregation for match-end checks

### Task 3: Implement free-for-all single-match flow

**Files:**
- Modify: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Modify: `Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`

Behavior:
- free-for-all starts as one playing phase, not a multi-round loop
- 5 kills ends the match immediately
- timeout with a unique leader ends the match immediately
- timeout tie enters `SuddenDeath`
- only tied leaders remain active; everyone else becomes spectating
- sudden death elimination resolves a unique winner and ends the match

### Task 4: Remove round-score HUD output

**Files:**
- Modify: `Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Modify: `Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`

Remove `RoundScoreText` from the widget tree and keep only `MatchScoreText`. Free-for-all summary should read as single-match progress rather than round progress.

### Task 5: Verify build and focused automation tests

Run:
- `Build.bat PvPArenaEditor Win64 Development`
- `Automation RunTests PvPArena.Match.WinCondition+PvPArena.Match.GameplayTempoDefaults+PvPArena.Match.EliminationRespawnBoundary+PvPArena.UI.HUDWidgetTeamScore+PvPArena.UI.HUDWidgetLayout`
