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
    TestFalse(
        TEXT("Elimination without a victim controller should not schedule respawn"),
        GameMode->ShouldScheduleRespawnAfterElimination(false));

    GameMode->ResolveRoundTimeout(3, 1);
    TestFalse(
        TEXT("Round-end elimination should not schedule respawn after winner is decided"),
        GameMode->ShouldScheduleRespawnAfterElimination(true));

    return true;
}
