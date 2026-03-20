# Best-Of-Five Match Flow Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a lobby, best-of-five round-win match flow, and final results screen so the first player to 3 round wins becomes the overall winner.

**Architecture:** Extend the existing single-map match flow by adding a replicated match-phase in `GameState`, replicated round-win counters in `PlayerState`, and a small amount of `GameMode` logic to transition between lobby, active play, round end, and final results. Reuse the existing HUD widget and layer two new overlays on top instead of creating separate maps.

**Tech Stack:** Unreal Engine 5 C++, replicated `GameMode`/`GameState`/`PlayerState`, UMG built in code, Automation tests.

---

### Task 1: Add failing score and state tests

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/ScoreReplicationDefaultsTest.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/PlayerStateRoundResetTest.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp`

**Step 1: Write failing tests**

- Add assertions for `RoundWins` defaulting to `0`.
- Add assertions that round reset preserves `RoundWins`.
- Add assertions that the first player to 3 round wins is treated as the final winner.

**Step 2: Run tests to verify they fail**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Match.ScoreDefaults+PvPArena.Match.PlayerStateRoundReset+PvPArena.Match.WinCondition; Quit'"`

Expected: FAIL on missing round-win behavior.

**Step 3: Write minimal implementation**

- Add replicated `RoundWins` and helpers to `APvPArenaPlayerState`.

**Step 4: Run tests to verify they pass**

Run the same automation command and expect PASS for score/player-state behaviors.

### Task 2: Add replicated match-phase and target score state

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Game/PvPArenaGameState.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Game/PvPArenaGameState.cpp`

**Step 1: Write the failing test**

- Extend match-flow tests to expect a replicated match-phase and a replicated target round-win count of `3`.

**Step 2: Run test to verify it fails**

Use the same test command or a dedicated new test if needed.

**Step 3: Write minimal implementation**

- Add `EPvPAMatchPhase`.
- Replicate current match phase and target round wins in `GameState`.

**Step 4: Run test to verify it passes**

Expect PASS.

### Task 3: Implement best-of-five game-mode transitions

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`
- Test: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp`

**Step 1: Write the failing test**

- Cover:
  - lobby as the initial phase
  - starting play once enough players are present
  - awarding round wins on round end
  - ending the whole match on 3 wins instead of starting another round

**Step 2: Run test to verify it fails**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Match.WinCondition; Quit'"`

Expected: FAIL.

**Step 3: Write minimal implementation**

- Start in `Lobby`.
- Detect when at least 2 players are present and begin a fresh match.
- Compute and award a round winner at kill-limit or timeout round end.
- If the winner reaches 3 round wins, move to `MatchEnd` and schedule lobby reset.
- Otherwise, continue existing next-round reset flow.

**Step 4: Run test to verify it passes**

Expect PASS.

### Task 4: Add lobby and result HUD overlays

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`
- Optionally modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`

**Step 1: Write the failing test**

- Add a HUD layout assertion if needed for new overlay widgets.

**Step 2: Run test to verify it fails**

Run the HUD test command if changed.

**Step 3: Write minimal implementation**

- Add `LobbyPanel` and `MatchResultPanel`.
- Show:
  - lobby waiting text while in `Lobby`
  - existing combat HUD while in `Playing`
  - final winner/result text while in `MatchEnd`
- Add round-win text to the status panel.

**Step 4: Run tests to verify they pass**

Run HUD tests if modified, otherwise verify by build + targeted gameplay test.

### Task 5: Verify end-to-end behavior

**Files:**
- No code changes unless defects are found.

**Step 1: Run build**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat' PvPArenaEditor Win64 Development '-Project=C:\UE5CPP\PvPArena\PvPArena.uproject' -WaitMutex -NoHotReloadFromIDE'"`

Expected: build succeeds.

**Step 2: Run automation coverage**

Run:
`powershell.exe -NoProfile -Command "& 'C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE5CPP\PvPArena\PvPArena.uproject' -unattended -nop4 -nosplash -NullRHI -NoSound '-ExecCmds=Automation RunTests PvPArena.Match.ScoreDefaults+PvPArena.Match.PlayerStateRoundReset+PvPArena.Match.WinCondition; Quit'"`

Expected: PASS.

**Step 3: Manual verification**

- Start PIE with 2 players.
- Confirm lobby shows before match start.
- Confirm match starts automatically with 2 players.
- Confirm a player reaching 3 round wins ends the match.
- Confirm result screen shows winner and round-win score.
- Confirm flow returns to lobby afterward.
