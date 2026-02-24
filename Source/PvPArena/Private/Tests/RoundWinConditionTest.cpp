#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameMode.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRoundWinConditionTest,
    "PvPArena.Match.WinCondition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRoundWinConditionTest::RunTest(const FString& Parameters)
{
    APvPArenaGameMode* GameMode = NewObject<APvPArenaGameMode>();
    TestNotNull(TEXT("GameMode should be created"), GameMode);

    if (!GameMode)
    {
        return false;
    }

    TestFalse(TEXT("No winner at start"), GameMode->HasWinner());
    return true;
}
