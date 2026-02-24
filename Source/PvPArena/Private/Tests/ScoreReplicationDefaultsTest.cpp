#include "Misc/AutomationTest.h"
#include "Game/PvPArenaPlayerState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FScoreDefaultsTest,
    "PvPArena.Match.ScoreDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FScoreDefaultsTest::RunTest(const FString& Parameters)
{
    APvPArenaPlayerState* PlayerState = NewObject<APvPArenaPlayerState>();
    TestNotNull(TEXT("PlayerState should be created"), PlayerState);

    if (!PlayerState)
    {
        return false;
    }

    TestEqual(TEXT("Kills default"), PlayerState->GetKills(), 0);
    TestEqual(TEXT("Deaths default"), PlayerState->GetDeaths(), 0);
    return true;
}
