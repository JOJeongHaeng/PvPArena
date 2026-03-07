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
    TestEqual(TEXT("Kills should increment"), PlayerState->GetKills(), 1);
    TestEqual(TEXT("Deaths should increment"), PlayerState->GetDeaths(), 1);

    PlayerState->ResetRoundStats();
    TestEqual(TEXT("Kills should reset"), PlayerState->GetKills(), 0);
    TestEqual(TEXT("Deaths should reset"), PlayerState->GetDeaths(), 0);
    return true;
}
