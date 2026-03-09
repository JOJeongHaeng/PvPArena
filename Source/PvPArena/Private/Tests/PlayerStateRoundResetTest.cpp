#include "Misc/AutomationTest.h"
#include "Game/PvPArenaPlayerState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPlayerStateRoundResetTest,
    "PvPArena.Match.PlayerStateRoundReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerStateRoundResetTest::RunTest(const FString& Parameters)
{
    APvPArenaPlayerState* PlayerState = NewObject<APvPArenaPlayerState>();
    TestNotNull(TEXT("PlayerState should be created"), PlayerState);

    if (!PlayerState)
    {
        return false;
    }

    PlayerState->AddKill();
    PlayerState->AddDeath();
    TestEqual(TEXT("Round kills should increment"), PlayerState->GetRoundKills(), 1);
    TestEqual(TEXT("Round deaths should increment"), PlayerState->GetRoundDeaths(), 1);
    TestEqual(TEXT("Match kills should increment"), PlayerState->GetMatchKills(), 1);
    TestEqual(TEXT("Match deaths should increment"), PlayerState->GetMatchDeaths(), 1);

    PlayerState->ResetRoundStats();
    TestEqual(TEXT("Round kills should reset"), PlayerState->GetRoundKills(), 0);
    TestEqual(TEXT("Round deaths should reset"), PlayerState->GetRoundDeaths(), 0);
    TestEqual(TEXT("Match kills should persist across round reset"), PlayerState->GetMatchKills(), 1);
    TestEqual(TEXT("Match deaths should persist across round reset"), PlayerState->GetMatchDeaths(), 1);
    return true;
}
