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

    TestEqual(TEXT("Round kills default"), PlayerState->GetRoundKills(), 0);
    TestEqual(TEXT("Round deaths default"), PlayerState->GetRoundDeaths(), 0);
    TestEqual(TEXT("Match kills default"), PlayerState->GetMatchKills(), 0);
    TestEqual(TEXT("Match deaths default"), PlayerState->GetMatchDeaths(), 0);
    return true;
}
