# Overhead Nameplate And Lobby Identity Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add replicated player nicknames, overhead nameplate health bars, and a lobby participant list that scales cleanly toward a future 6-player lobby.

**Architecture:** `APvPArenaPlayerState` will own a replicated display nickname and server-side default-name normalization. `APvPArenaCharacter` will host a world-space widget component for nickname and health, while `UPvPArenaHUDWidget` will add nickname entry and a participant list driven from `APvPArenaGameState::PlayerArray`.

**Tech Stack:** Unreal Engine 5 C++, `APlayerState` replication, `UWidgetComponent`, `UUserWidget`, existing automation tests.

---

### Task 1: Replicated nickname source of truth

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Game/PvPArenaPlayerState.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Game/PvPArenaPlayerState.cpp`
- Test: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/ScoreReplicationDefaultsTest.cpp`
- Create: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/PlayerStateDisplayNameTest.cpp`

**Step 1: Write the failing test**

Add `PlayerStateDisplayNameTest.cpp` covering:

```cpp
APvPArenaPlayerState* PlayerState = NewObject<APvPArenaPlayerState>();
PlayerState->SetDisplayNickname(TEXT("   "));
TestEqual(TEXT("Blank nickname should normalize to empty request"), PlayerState->BuildNormalizedNickname(TEXT("   ")), FString());
```

Also verify a stored fallback such as `Player3` is returned by a new getter when set.

**Step 2: Run test to verify it fails**

Run: `UnrealEditor-Cmd ... -ExecCmds="Automation RunTests PvPArena.Match.PlayerStateDisplayName; Quit"`

Expected: FAIL because nickname helpers and replicated property do not exist yet.

**Step 3: Write minimal implementation**

Add to `APvPArenaPlayerState`:

```cpp
FString GetDisplayNickname() const;
void SetDisplayNickname(const FString& NewNickname);
static FString BuildNormalizedNickname(const FString& RawNickname);
```

Add a replicated `FString DisplayNickname`, normalize via trim, and include it in `GetLifetimeReplicatedProps`.

**Step 4: Run test to verify it passes**

Run the same automation test and expect PASS.

**Step 5: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Public/Game/PvPArenaPlayerState.h Source/PvPArena/Private/Game/PvPArenaPlayerState.cpp Source/PvPArena/Private/Tests/PlayerStateDisplayNameTest.cpp Source/PvPArena/Private/Tests/ScoreReplicationDefaultsTest.cpp
git -C /mnt/c/UE5CPP/PvPArena commit -m "feat: add replicated display nicknames"
```

### Task 2: Nickname submission from lobby

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Game/PvPArenaPlayerController.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Game/PvPArenaPlayerController.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`
- Test: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/PlayerStateDisplayNameTest.cpp`

**Step 1: Write the failing test**

Extend `PlayerStateDisplayNameTest.cpp` with a server-side helper expectation:

```cpp
TestEqual(TEXT("Empty nickname should resolve to Player1"), APvPArenaGameMode::BuildDefaultDisplayNickname(1), FString(TEXT("Player1")));
TestEqual(TEXT("Sixth fallback nickname should resolve cleanly"), APvPArenaGameMode::BuildDefaultDisplayNickname(6), FString(TEXT("Player6")));
```

**Step 2: Run test to verify it fails**

Run the nickname automation test.

Expected: FAIL because the helper and submission path do not exist.

**Step 3: Write minimal implementation**

Add a player-controller RPC such as:

```cpp
UFUNCTION(BlueprintCallable, Category = "Lobby")
void SubmitLobbyNickname(const FString& Nickname);
```

Route it to game mode logic that:

- trims the string
- keeps a non-empty custom nickname
- assigns the next unique fallback `PlayerN` when empty
- writes the final value into the player's `APvPArenaPlayerState`

Expose a static `BuildDefaultDisplayNickname(int32 PlayerIndex)` helper for deterministic tests.

**Step 4: Run test to verify it passes**

Run the nickname automation test and expect PASS.

**Step 5: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Public/Game/PvPArenaPlayerController.h Source/PvPArena/Private/Game/PvPArenaPlayerController.cpp Source/PvPArena/Public/Game/PvPArenaGameMode.h Source/PvPArena/Private/Game/PvPArenaGameMode.cpp Source/PvPArena/Private/Tests/PlayerStateDisplayNameTest.cpp
git -C /mnt/c/UE5CPP/PvPArena commit -m "feat: submit lobby nicknames"
```

### Task 3: Overhead widget display

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`
- Create: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/UI/PvPArenaOverheadStatusWidget.h`
- Create: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/UI/PvPArenaOverheadStatusWidget.cpp`
- Create: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/OverheadStatusWidgetTest.cpp`

**Step 1: Write the failing test**

Create `OverheadStatusWidgetTest.cpp` covering a helper like:

```cpp
FString NicknameLabel;
float HealthPercent = 0.0f;
UPvPArenaOverheadStatusWidget::BuildDisplayState(Character, NicknameLabel, HealthPercent);
TestEqual(TEXT("Nickname should come from player state"), NicknameLabel, FString(TEXT("Player2")));
TestEqual(TEXT("Health percent should match current health"), HealthPercent, 0.5f);
```

**Step 2: Run test to verify it fails**

Run: `Automation RunTests PvPArena.UI.OverheadStatusWidget`

Expected: FAIL because the widget class and helper do not exist.

**Step 3: Write minimal implementation**

Create a lightweight widget with:

```cpp
static void BuildDisplayState(const APvPArenaCharacter* Character, FString& OutNickname, float& OutHealthPercent);
```

In `APvPArenaCharacter`, add a `UWidgetComponent` attached above the mesh, assign `UPvPArenaOverheadStatusWidget::StaticClass()`, and refresh it from `BeginPlay`, `PossessedBy`, `OnRep_Controller`, and `OnRep_CurrentHealth`.

**Step 4: Run test to verify it passes**

Run the new widget automation test and expect PASS.

**Step 5: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Public/Player/PvPArenaCharacter.h Source/PvPArena/Private/Player/PvPArenaCharacter.cpp Source/PvPArena/Public/UI/PvPArenaOverheadStatusWidget.h Source/PvPArena/Private/UI/PvPArenaOverheadStatusWidget.cpp Source/PvPArena/Private/Tests/OverheadStatusWidgetTest.cpp
git -C /mnt/c/UE5CPP/PvPArena commit -m "feat: add overhead status widget"
```

### Task 4: Lobby nickname field and participant list

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`
- Create: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetLobbyPlayerListTest.cpp`

**Step 1: Write the failing test**

Create `HUDWidgetLobbyPlayerListTest.cpp` with helper expectations such as:

```cpp
TArray<FString> ParticipantLabels = UPvPArenaHUDWidget::BuildLobbyParticipantLabels(GameState);
TestEqual(TEXT("Participant count should match player array"), ParticipantLabels.Num(), 3);
TestEqual(TEXT("First participant should show fallback nickname"), ParticipantLabels[0], FString(TEXT("Player1")));
```

Update layout tests to expect a nickname text box and participant list container in the lobby tree.

**Step 2: Run test to verify it fails**

Run the HUD layout and lobby list automation tests.

Expected: FAIL because those widgets and helpers do not exist.

**Step 3: Write minimal implementation**

Add a `LobbyNicknameTextBox` and a participant-list container to the lobby panel. Refresh the list from `GameState->PlayerArray`, prefill the text box from the local player's replicated nickname, and submit changed text through `APvPArenaPlayerController::SubmitLobbyNickname`.

**Step 4: Run test to verify it passes**

Run the HUD automation tests and expect PASS.

**Step 5: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add Source/PvPArena/Public/UI/PvPArenaHUDWidget.h Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp Source/PvPArena/Private/Tests/HUDWidgetLobbyPlayerListTest.cpp
git -C /mnt/c/UE5CPP/PvPArena commit -m "feat: show lobby nicknames and participants"
```

### Task 5: End-to-end verification

**Files:**
- Verify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/PlayerStateDisplayNameTest.cpp`
- Verify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/OverheadStatusWidgetTest.cpp`
- Verify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetLobbyPlayerListTest.cpp`
- Verify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetLayoutTest.cpp`

**Step 1: Run the targeted tests**

Run the new nickname, overhead widget, and HUD tests together.

**Step 2: Build the editor target**

Run the approved UE build command for `PvPArenaEditor Win64 Development`.

**Step 3: Run the final automation batch**

Run:

```bash
Automation RunTests PvPArena.Match.PlayerStateDisplayName+PvPArena.UI.OverheadStatusWidget+PvPArena.UI.HUDWidgetLobbyPlayerList+PvPArena.UI.HUDWidgetLayout
```

Expected: all tests report `Result={Success}`.

**Step 4: Commit**

```bash
git -C /mnt/c/UE5CPP/PvPArena add docs/plans/2026-03-31-overhead-nameplate-design.md docs/plans/2026-03-31-overhead-nameplate-and-lobby-identity.md
git -C /mnt/c/UE5CPP/PvPArena commit -m "docs: add overhead nameplate implementation plan"
```
