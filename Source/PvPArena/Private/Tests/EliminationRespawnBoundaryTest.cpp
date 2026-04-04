#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameMode.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FEliminationRespawnBoundaryTest,
    "PvPArena.Match.EliminationRespawnBoundary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEliminationRespawnBoundaryTest::RunTest(const FString& Parameters)
{
    APvPArenaGameMode* GameMode = NewObject<APvPArenaGameMode>();
    TestNotNull(TEXT("GameMode should be created"), GameMode);

    if (!GameMode)
    {
        return false;
    }

    TestTrue(
        TEXT("Active round elimination with a victim should schedule respawn"),
        GameMode->ShouldScheduleRespawnAfterElimination(true));
    TestTrue(
        TEXT("Free-for-all should continue to respawn eliminated players during an active round"),
        GameMode->ShouldScheduleRespawnAfterEliminationForMode(EPvPALobbyMatchMode::FreeForAll, true, false));
    TestFalse(
        TEXT("Team-versus should not respawn eliminated players until the round ends"),
        GameMode->ShouldScheduleRespawnAfterEliminationForMode(EPvPALobbyMatchMode::TeamVersus, true, false));
    TestTrue(
        TEXT("Team-versus eliminations should immediately enter spectating"),
        GameMode->ShouldEnterSpectatingAfterEliminationForMode(EPvPALobbyMatchMode::TeamVersus, true, false));
    TestFalse(
        TEXT("Free-for-all sudden death should not respawn eliminated players while leaders are deciding first place"),
        GameMode->ShouldScheduleRespawnAfterEliminationForMode(EPvPALobbyMatchMode::FreeForAll, true, false, EPvPARoundState::SuddenDeath));
    TestTrue(
        TEXT("Free-for-all sudden death eliminations should enter spectating"),
        GameMode->ShouldEnterSpectatingAfterEliminationForMode(EPvPALobbyMatchMode::FreeForAll, true, false, EPvPARoundState::SuddenDeath));
    TestFalse(
        TEXT("Regular free-for-all eliminations should not enter spectating because they respawn"),
        GameMode->ShouldEnterSpectatingAfterEliminationForMode(EPvPALobbyMatchMode::FreeForAll, true, false));
    TestFalse(
        TEXT("Elimination without a victim controller should not schedule respawn"),
        GameMode->ShouldScheduleRespawnAfterElimination(false));
    TestFalse(
        TEXT("A finished round should never schedule another respawn regardless of mode"),
        GameMode->ShouldScheduleRespawnAfterEliminationForMode(EPvPALobbyMatchMode::FreeForAll, true, true));

    GameMode->ResolveRoundTimeout(3, 1);
    TestFalse(
        TEXT("Round-end elimination should not schedule respawn after winner is decided"),
        GameMode->ShouldScheduleRespawnAfterElimination(true));

    return true;
}
