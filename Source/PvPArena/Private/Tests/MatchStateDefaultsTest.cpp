#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMatchStateDefaultsTest,
    "PvPArena.Match.MatchStateDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchStateDefaultsTest::RunTest(const FString& Parameters)
{
    APvPArenaGameState* GameState = NewObject<APvPArenaGameState>();
    TestNotNull(TEXT("GameState should be created"), GameState);

    if (!GameState)
    {
        return false;
    }

    TestEqual(TEXT("Default match phase should begin in lobby"), GameState->GetMatchPhase(), EPvPAMatchPhase::Lobby);
    TestEqual(TEXT("Default round wins-to-win should require three wins"), GameState->GetRoundWinsToWin(), 3);
    TestEqual(TEXT("Default lobby countdown should start empty"), GameState->GetRemainingLobbyCountdownSeconds(), 0);
    TestEqual(TEXT("Default match-end timer should start empty"), GameState->GetRemainingMatchEndTimeSeconds(), 0);
    TestNull(TEXT("Default round winner should be empty"), GameState->GetRoundWinner());
    TestNull(TEXT("Default match winner should be empty"), GameState->GetMatchWinner());
    return true;
}
