# HUD Ownership Cleanup Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Make `APvPArenaPlayerController` the sole runtime owner of the HUD widget so respawn and possession changes cannot create duplicate HUDs.

**Architecture:** Keep widget construction in the local player controller and remove the character-side HUD creation path entirely. Preserve HUD data refresh inside `UPvPArenaHUDWidget`, since it already reads current pawn, player state, and game state from the owning player each refresh tick.

**Tech Stack:** Unreal Engine 5.5, C++, Unreal Automation Tests, Listen Server PIE

---

### Task 1: Lock HUD ownership to the player controller

**Files:**
- Create: `Source/PvPArena/Private/Tests/PlayerControllerHUDDefaultsTest.cpp`
- Modify: `Source/PvPArena/Public/Game/PvPArenaPlayerController.h`
- Modify: `Source/PvPArena/Private/Game/PvPArenaPlayerController.cpp`
- Modify: `Source/PvPArena/Public/Player/PvPArenaCharacter.h`
- Modify: `Source/PvPArena/Private/Player/PvPArenaCharacter.cpp`

**Step 1: Write the failing test**

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPlayerControllerHUDDefaultsTest,
    "PvPArena.UI.PlayerControllerHUDDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerControllerHUDDefaultsTest::RunTest(const FString& Parameters)
{
    const APvPArenaPlayerController* PlayerController = GetDefault<APvPArenaPlayerController>();
    TestNotNull(TEXT("PlayerController CDO should exist"), PlayerController);
    const FClassProperty* HUDClassProperty = FindFProperty<FClassProperty>(APvPArenaPlayerController::StaticClass(), TEXT("HUDWidgetClass"));
    const UClass* HUDClassValue = HUDClassProperty
        ? Cast<UClass>(HUDClassProperty->GetPropertyValue_InContainer(PlayerController))
        : nullptr;
    TestTrue(TEXT("PlayerController should default to a HUD widget class"),
        HUDClassValue == UPvPArenaHUDWidget::StaticClass());
    return true;
}
```

**Step 2: Run test to verify it fails**

Run: project build, then automation test `PvPArena.UI.PlayerControllerHUDDefaults`  
Expected: FAIL because controller has no explicit default HUD class yet.

**Step 3: Write minimal implementation**

```cpp
APvPArenaPlayerController::APvPArenaPlayerController()
{
    HUDWidgetClass = UPvPArenaHUDWidget::StaticClass();
}
```

Remove HUD creation fields/methods from `APvPArenaCharacter`.

**Step 4: Run test to verify it passes**

Run the same build and automation test.  
Expected: PASS.

**Step 5: Commit**

```bash
git add docs/plans/2026-03-09-hud-ownership-cleanup.md Source/PvPArena/Public/Game/PvPArenaPlayerController.h Source/PvPArena/Private/Game/PvPArenaPlayerController.cpp Source/PvPArena/Public/Player/PvPArenaCharacter.h Source/PvPArena/Private/Player/PvPArenaCharacter.cpp Source/PvPArena/Private/Tests/PlayerControllerHUDDefaultsTest.cpp
git commit -m "refactor: centralize hud ownership in player controller"
```
