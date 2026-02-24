# PvPArena Standard MVP Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a playable 1v1 Listen Server PvP MVP (melee + ranged, HP/death/respawn, score/timer/win states, basic HUD) on top of ThirdPerson template.

**Architecture:** Gameplay authority lives in C++ (GameMode/GameState/PlayerState/Character/CombatComponent) with server-authoritative combat and replicated state. Blueprint is used for presentation (animations, effects, HUD widgets) and data wiring to C++ events. Iteration follows strict TDD per vertical slice with small commits.

**Tech Stack:** Unreal Engine 5.5, C++, Blueprint/UMG, Unreal Automation Tests (Editor context), Listen Server PIE.

---

### Task 1: Convert BP-only project to C++ baseline

**Files:**
- Create: `Source/PvPArena/PvPArena.Build.cs`
- Create: `Source/PvPArena/PvPArena.cpp`
- Create: `Source/PvPArena/PvPArena.h`
- Create: `Source/PvPArena.Target.cs`
- Create: `Source/PvPArenaEditor.Target.cs`

**Step 1: Write the failing test**

```cpp
// Source/PvPArena/Private/Tests/ModuleBootstrapTest.cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPvPArenaModuleBootstrapTest,
    "PvPArena.Bootstrap.ModuleLoads",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPvPArenaModuleBootstrapTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("Placeholder failing assert until module created"), false);
    return true;
}
```

**Step 2: Run test to verify it fails**

Run: `Build.bat PvPArenaEditor Win64 Development -Project="C:\UE5CPP\PvPArena\PvPArena.uproject" -WaitMutex -NoHotReloadFromIDE` then run automation test `PvPArena.Bootstrap.ModuleLoads` in Editor.  
Expected: FAIL (false assert or missing compile unit before C++ baseline exists).

**Step 3: Write minimal implementation**

```cpp
// Source/PvPArena/PvPArena.cpp
#include "PvPArena.h"
#include "Modules/ModuleManager.h"
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, PvPArena, "PvPArena");
```

**Step 4: Run test to verify it passes**

Run same build and automation test.  
Expected: PASS after replacing failing placeholder with a true module sanity assertion.

**Step 5: Commit**

```bash
git add Source/PvPArena Source/PvPArena.Target.cs Source/PvPArenaEditor.Target.cs
git commit -m "chore: bootstrap c++ module for pvp arena"
```

### Task 2: Add replicated player combat state (health/alive)

**Files:**
- Create: `Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Create: `Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`
- Create: `Source/PvPArena/Private/Tests/CharacterHealthReplicationTest.cpp`

**Step 1: Write the failing test**

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterHealthDefaultsTest,
    "PvPArena.Character.HealthDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterHealthDefaultsTest::RunTest(const FString& Parameters)
{
    APvPArenaCharacter* Char = NewObject<APvPArenaCharacter>();
    TestEqual(TEXT("Default max health"), Char->GetMaxHealth(), 100.f);
    TestEqual(TEXT("Default current health"), Char->GetCurrentHealth(), 100.f);
    return true;
}
```

**Step 2: Run test to verify it fails**

Run: `Build.bat ...` then automation test `PvPArena.Character.HealthDefaults`.  
Expected: FAIL (class/getters missing).

**Step 3: Write minimal implementation**

```cpp
UPROPERTY(EditDefaultsOnly, ReplicatedUsing=OnRep_CurrentHealth)
float CurrentHealth = 100.f;
UPROPERTY(EditDefaultsOnly)
float MaxHealth = 100.f;

UFUNCTION()
void OnRep_CurrentHealth();
```

Add `GetLifetimeReplicatedProps` and const getters.

**Step 4: Run test to verify it passes**

Run build + `PvPArena.Character.HealthDefaults`.  
Expected: PASS.

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Player/PvPArenaCharacter.h Source/PvPArena/Private/Player/PvPArenaCharacter.cpp Source/PvPArena/Private/Tests/CharacterHealthReplicationTest.cpp
git commit -m "feat: add replicated health state to character"
```

### Task 3: Implement server-authoritative damage/death and respawn timer hooks

**Files:**
- Modify: `Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`
- Create: `Source/PvPArena/Private/Tests/CharacterDeathFlowTest.cpp`

**Step 1: Write the failing test**

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterDeathFlowTest,
    "PvPArena.Character.DeathFlow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterDeathFlowTest::RunTest(const FString& Parameters)
{
    APvPArenaCharacter* Char = NewObject<APvPArenaCharacter>();
    Char->ApplyServerDamage(150.f, nullptr);
    TestTrue(TEXT("Character should be dead"), Char->IsDead());
    return true;
}
```

**Step 2: Run test to verify it fails**

Run `PvPArena.Character.DeathFlow`.  
Expected: FAIL (damage/death API missing).

**Step 3: Write minimal implementation**

```cpp
void APvPArenaCharacter::ApplyServerDamage(float Damage, AController* InstigatorController)
{
    if (!HasAuthority() || bIsDead) return;
    CurrentHealth = FMath::Max(0.f, CurrentHealth - Damage);
    if (CurrentHealth <= 0.f) { HandleDeath(InstigatorController); }
}
```

Add `bIsDead` replication and death delegate for GameMode respawn scheduling.

**Step 4: Run test to verify it passes**

Run build + `PvPArena.Character.DeathFlow`.  
Expected: PASS.

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Player/PvPArenaCharacter.h Source/PvPArena/Private/Player/PvPArenaCharacter.cpp Source/PvPArena/Private/Tests/CharacterDeathFlowTest.cpp
git commit -m "feat: implement server damage and death state"
```

### Task 4: Add combat component with melee + ranged server validation

**Files:**
- Create: `Source/PvPArena/Public/Combat/PvPCombatComponent.h`
- Create: `Source/PvPArena/Private/Combat/PvPCombatComponent.cpp`
- Modify: `Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`
- Create: `Source/PvPArena/Private/Tests/CombatCooldownValidationTest.cpp`

**Step 1: Write the failing test**

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatCooldownTest,
    "PvPArena.Combat.Cooldown",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatCooldownTest::RunTest(const FString& Parameters)
{
    UPvPCombatComponent* Combat = NewObject<UPvPCombatComponent>();
    TestTrue(TEXT("First melee use allowed"), Combat->CanUseMelee(0.f));
    Combat->MarkMeleeUsed(0.f);
    TestFalse(TEXT("Immediate reuse blocked"), Combat->CanUseMelee(0.1f));
    return true;
}
```

**Step 2: Run test to verify it fails**

Run `PvPArena.Combat.Cooldown`.  
Expected: FAIL.

**Step 3: Write minimal implementation**

```cpp
bool UPvPCombatComponent::CanUseMelee(float Now) const { return Now >= NextAllowedMeleeTime; }
void UPvPCombatComponent::MarkMeleeUsed(float Now) { NextAllowedMeleeTime = Now + MeleeCooldownSeconds; }
```

Add server RPC entry points from character:
- `ServerTryMeleeAttack()`
- `ServerTryRangedAttack()`

**Step 4: Run test to verify it passes**

Run build + `PvPArena.Combat.Cooldown`.  
Expected: PASS.

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Combat/PvPCombatComponent.h Source/PvPArena/Private/Combat/PvPCombatComponent.cpp Source/PvPArena/Public/Player/PvPArenaCharacter.h Source/PvPArena/Private/Player/PvPArenaCharacter.cpp Source/PvPArena/Private/Tests/CombatCooldownValidationTest.cpp
git commit -m "feat: add combat component with attack cooldown validation"
```

### Task 5: Add PlayerState and GameState replicated match data

**Files:**
- Create: `Source/PvPArena/Public/Game/PvPArenaPlayerState.h`
- Create: `Source/PvPArena/Private/Game/PvPArenaPlayerState.cpp`
- Create: `Source/PvPArena/Public/Game/PvPArenaGameState.h`
- Create: `Source/PvPArena/Private/Game/PvPArenaGameState.cpp`
- Create: `Source/PvPArena/Private/Tests/ScoreReplicationDefaultsTest.cpp`

**Step 1: Write the failing test**

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScoreDefaultsTest,
    "PvPArena.Match.ScoreDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FScoreDefaultsTest::RunTest(const FString& Parameters)
{
    APvPArenaPlayerState* PS = NewObject<APvPArenaPlayerState>();
    TestEqual(TEXT("Kills default"), PS->GetKills(), 0);
    TestEqual(TEXT("Deaths default"), PS->GetDeaths(), 0);
    return true;
}
```

**Step 2: Run test to verify it fails**

Run `PvPArena.Match.ScoreDefaults`.  
Expected: FAIL.

**Step 3: Write minimal implementation**

```cpp
UPROPERTY(ReplicatedUsing=OnRep_Kills) int32 Kills = 0;
UPROPERTY(ReplicatedUsing=OnRep_Deaths) int32 Deaths = 0;
```

In GameState add replicated:
- `RemainingRoundTimeSeconds`
- `RoundState`
- `ScoreLimit`

**Step 4: Run test to verify it passes**

Run build + `PvPArena.Match.ScoreDefaults`.  
Expected: PASS.

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Game/PvPArenaPlayerState.h Source/PvPArena/Private/Game/PvPArenaPlayerState.cpp Source/PvPArena/Public/Game/PvPArenaGameState.h Source/PvPArena/Private/Game/PvPArenaGameState.cpp Source/PvPArena/Private/Tests/ScoreReplicationDefaultsTest.cpp
git commit -m "feat: add replicated player and match score state"
```

### Task 6: Implement GameMode round loop and win conditions

**Files:**
- Create: `Source/PvPArena/Public/Game/PvPArenaGameMode.h`
- Create: `Source/PvPArena/Private/Game/PvPArenaGameMode.cpp`
- Modify: `Config/DefaultGame.ini`
- Create: `Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp`

**Step 1: Write the failing test**

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRoundWinConditionTest,
    "PvPArena.Match.WinCondition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRoundWinConditionTest::RunTest(const FString& Parameters)
{
    APvPArenaGameMode* GM = NewObject<APvPArenaGameMode>();
    TestFalse(TEXT("No winner at start"), GM->HasWinner());
    return true;
}
```

**Step 2: Run test to verify it fails**

Run `PvPArena.Match.WinCondition`.  
Expected: FAIL.

**Step 3: Write minimal implementation**

```cpp
UPROPERTY(EditDefaultsOnly) int32 ScoreLimit = 5;
UPROPERTY(EditDefaultsOnly) int32 RoundDurationSeconds = 180;
UPROPERTY(EditDefaultsOnly) int32 RespawnDelaySeconds = 3;
```

Implement:
- kill update callback
- timeout evaluation
- sudden death transition on tie
- round reset/start

**Step 4: Run test to verify it passes**

Run build + `PvPArena.Match.WinCondition`.  
Expected: PASS.

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/Game/PvPArenaGameMode.h Source/PvPArena/Private/Game/PvPArenaGameMode.cpp Config/DefaultGame.ini Source/PvPArena/Private/Tests/RoundWinConditionTest.cpp
git commit -m "feat: implement round flow and win condition logic"
```

### Task 7: Wire HUD data interface and Blueprint widget bindings

**Files:**
- Create: `Source/PvPArena/Public/UI/PvPArenaHUDDataComponent.h`
- Create: `Source/PvPArena/Private/UI/PvPArenaHUDDataComponent.cpp`
- Modify: `Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`
- Modify: `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.uasset`
- Create: `Content/UI/WBP_PvPArenaHUD.uasset`

**Step 1: Write the failing test**

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDDataExposureTest,
    "PvPArena.UI.HUDDataExposure",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDDataExposureTest::RunTest(const FString& Parameters)
{
    UPvPArenaHUDDataComponent* HUDData = NewObject<UPvPArenaHUDDataComponent>();
    TestEqual(TEXT("Initial displayed health"), HUDData->GetDisplayedHealth(), 100.f);
    return true;
}
```

**Step 2: Run test to verify it fails**

Run `PvPArena.UI.HUDDataExposure`.  
Expected: FAIL.

**Step 3: Write minimal implementation**

Expose Blueprint-callable getters and multicast delegates:
- `OnHealthChanged`
- `OnMatchStateChanged`
- `OnRespawnCountdownChanged`

Bind UMG widget to events instead of per-frame tick polling.

**Step 4: Run test to verify it passes**

Run build + `PvPArena.UI.HUDDataExposure`.  
Expected: PASS.

**Step 5: Commit**

```bash
git add Source/PvPArena/Public/UI/PvPArenaHUDDataComponent.h Source/PvPArena/Private/UI/PvPArenaHUDDataComponent.cpp Source/PvPArena/Public/Player/PvPArenaCharacter.h Source/PvPArena/Private/Player/PvPArenaCharacter.cpp Content/UI/WBP_PvPArenaHUD.uasset Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.uasset
git commit -m "feat: add event-driven hud data bridge and widget"
```

### Task 8: End-to-end multiplayer verification and documentation

**Files:**
- Create: `docs/testing/pvp-mvp-smoke-checklist.md`
- Modify: `docs/plans/2026-02-24-pvp-mvp-design.md`

**Step 1: Write the failing test**

```text
Manual smoke test entry starts as FAIL until all scenarios pass.
```

**Step 2: Run test to verify it fails**

Run PIE with 2 players and execute checklist; mark current failures.  
Expected: At least one fail before bugfix pass.

**Step 3: Write minimal implementation**

Fix discovered issues only (YAGNI), then document exact repro and pass criteria in checklist.

**Step 4: Run test to verify it passes**

Re-run 2-player checklist:
- movement/jump/melee/ranged
- death/respawn sync
- score/timer/state sync
- score-limit win
- timeout win
- sudden death

Expected: All PASS.

**Step 5: Commit**

```bash
git add docs/testing/pvp-mvp-smoke-checklist.md docs/plans/2026-02-24-pvp-mvp-design.md
git commit -m "test: add mvp multiplayer smoke checklist and validation notes"
```
