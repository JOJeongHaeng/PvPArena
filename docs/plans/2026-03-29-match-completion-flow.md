# Match Completion Flow Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a complete match loop to PvPArena with round-win tracking, best-of-three match completion, final result presentation, and automatic return to lobby.

**Architecture:** Keep the existing round-state system and add a higher-level replicated match phase in `GameState`. Store per-player round wins in `PlayerState`, let `GameMode` award round wins and switch between `Lobby`, `Playing`, and `MatchEnd`, and extend the existing code-built HUD to show round wins plus a final result overlay.

**Tech Stack:** Unreal Engine 5 C++, replicated `GameMode` / `GameState` / `PlayerState`, code-built UMG HUD, Automation tests

---

### Task 1: Add failing round-win state tests

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/ScoreReplicationDefaultsTest.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/PlayerStateRoundResetTest.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Game/PvPArenaPlayerState.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Game/PvPArenaPlayerState.cpp`

**Step 1: Write the failing test**

- Extend `ScoreReplicationDefaultsTest.cpp` to assert `RoundWins` defaults to `0`.
- Extend `PlayerStateRoundResetTest.cpp` to assert round-local reset preserves `RoundWins`.
- Add a second assertion that full match reset clears `RoundWins`.

**Step 2: Run test to verify it fails**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Match.ScoreDefaults+PvPArena.Match.PlayerStateRoundReset; Quit'"`

Expected: FAIL on missing `RoundWins` state/reset behavior.

**Step 3: Write minimal implementation**

- Add replicated `int32 RoundWins = 0;` to `APvPArenaPlayerState`.
- Add helpers for `AddRoundWin()`, `ResetRoundStats()`, and `ResetMatchStats()`.
- Keep round reset and full match reset responsibilities separate.

**Step 4: Run test to verify it passes**

Run the same automation command.

Expected: PASS.

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Game/PvPArenaPlayerState.h Source/PvPArena/Private/Game/PvPArenaPlayerState.cpp Source/PvPArena/Private/Tests/ScoreReplicationDefaultsTest.cpp Source/PvPArena/Private/Tests/PlayerStateRoundResetTest.cpp
git commit -m "feat: add round win state"
```

### Task 2: Add replicated match phase and target round wins

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Game/PvPArenaGameState.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Game/PvPArenaGameState.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/MatchStateDefaultsTest.cpp`

**Step 1: Write the failing test**

- Extend `MatchStateDefaultsTest.cpp` to assert:
  - match phase defaults to `Lobby`
  - target round wins defaults to `2` for a best-of-three flow
  - result state starts empty or neutral

**Step 2: Run test to verify it fails**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Match.MatchStateDefaults; Quit'"`

Expected: FAIL on missing match-phase state.

**Step 3: Write minimal implementation**

- Add `EPvPAMatchPhase` enum with `Lobby`, `Playing`, `MatchEnd`.
- Add replicated `CurrentMatchPhase`.
- Add replicated `TargetRoundWins = 2`.
- Add optional winner/result text storage only if needed by HUD/tests.

**Step 4: Run test to verify it passes**

Run the same automation command.

Expected: PASS.

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Game/PvPArenaGameState.h Source/PvPArena/Private/Game/PvPArenaGameState.cpp Source/PvPArena/Private/Tests/MatchStateDefaultsTest.cpp
git commit -m "feat: add match phase state"
```

### Task 3: Add failing match completion rules

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`

**Step 1: Write the failing test**

- Extend `RoundWinConditionTest.cpp` to cover:
  - initial `Lobby` phase
  - transition to `Playing` when enough players are present
  - awarding a round win to the correct player at round end
  - ending the match at 2 round wins
  - preventing the next-round loop when the match is over

**Step 2: Run test to verify it fails**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Match.WinCondition; Quit'"`

Expected: FAIL on missing match completion behavior.

**Step 3: Write minimal implementation**

- In `GameMode`, start in `Lobby`.
- Add a check for minimum player count before beginning the match.
- At round end, determine the winner using existing kill-limit / timeout logic.
- Award `RoundWins` to the winner.
- If winner `RoundWins >= TargetRoundWins`, switch to `MatchEnd`.
- Otherwise continue existing next-round reset flow.

**Step 4: Run test to verify it passes**

Run the same automation command.

Expected: PASS.

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Game/PvPArenaGameMode.h Source/PvPArena/Private/Game/PvPArenaGameMode.cpp Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp
git commit -m "feat: add best-of-three match completion"
```

### Task 4: Add result display and round-win HUD

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`

**Step 1: Write the failing test**

- Extend `HUDWidgetLayoutTest.cpp` to assert:
  - a match-score / round-win text widget exists
  - a match-result panel exists
  - the result panel is hidden by default

**Step 2: Run test to verify it fails**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.UI.HUDWidgetLayout; Quit'"`

Expected: FAIL on missing HUD/result widgets or behavior.

**Step 3: Write minimal implementation**

- Show each player's round wins in the existing status/info area.
- Populate `MatchResultPanel` from the replicated match phase/winner state.
- While `Lobby`, show lobby text.
- While `Playing`, show combat HUD.
- While `MatchEnd`, show result panel and suppress combat-only messaging if needed.

**Step 4: Run test to verify it passes**

Run the same automation command.

Expected: PASS.

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/UI/PvPArenaHUDWidget.h Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp
git commit -m "feat: add match completion HUD"
```

### Task 5: Return from match end to lobby

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp`

**Step 1: Write the failing test**

- Extend `RoundWinConditionTest.cpp` to assert:
  - after `MatchEnd`, a timer or reset path returns the game to `Lobby`
  - match reset clears `RoundWins`
  - round-local and match-local stats are both reset for the next match

**Step 2: Run test to verify it fails**

Run the same automation command from Task 3.

Expected: FAIL on missing lobby reset behavior.

**Step 3: Write minimal implementation**

- Add a short match-end delay timer in `GameMode`.
- Reset winner/result state.
- Clear player match stats and round wins.
- Transition back to `Lobby`.
- Wait for minimum player count to start the next match.

**Step 4: Run test to verify it passes**

Run the same automation command from Task 3.

Expected: PASS.

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Game/PvPArenaGameMode.h Source/PvPArena/Private/Game/PvPArenaGameMode.cpp Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp
git commit -m "feat: reset completed matches to lobby"
```

### Task 6: Full verification

**Files:**
- No code changes unless defects are found.

**Step 1: Run targeted automation**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Match.ScoreDefaults+PvPArena.Match.PlayerStateRoundReset+PvPArena.Match.MatchStateDefaults+PvPArena.Match.WinCondition+PvPArena.UI.HUDWidgetLayout; Quit'"`

Expected: PASS.

**Step 2: Run build**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat' PvPArenaEditor Win64 Development '-Project=C:\UE5CPP\PvPArena\PvPArena.uproject' -WaitMutex -NoHotReloadFromIDE'"`

Expected: build succeeds.

**Step 3: Manual verification**

- Launch PIE with 2 players.
- Confirm the game starts in `Lobby`.
- Confirm the match begins when enough players are present.
- Confirm the HUD shows round wins.
- Confirm a player reaching 2 round wins triggers the result screen.
- Confirm the match returns to `Lobby` after the result delay.

**Step 4: Commit final fixes if needed**

```bash
git add <files changed during verification>
git commit -m "test: verify match completion flow"
```
