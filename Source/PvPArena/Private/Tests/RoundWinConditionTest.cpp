#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameMode.h"
#include "Game/PvPArenaPlayerState.h"

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
    TestEqual(TEXT("Matches should begin from the lobby phase"), GameMode->GetInitialMatchPhase(), EPvPAMatchPhase::Lobby);
    TestEqual(
        TEXT("Tie on timeout should enter sudden death"),
        GameMode->ResolveRoundTimeout(3, 3),
        EPvPARoundState::SuddenDeath);
    TestFalse(TEXT("Sudden death state should not mark winner"), GameMode->HasWinner());

    TestEqual(
        TEXT("Non-tie timeout should end round"),
        GameMode->ResolveRoundTimeout(4, 2),
        EPvPARoundState::RoundEnd);
    TestTrue(TEXT("Non-tie timeout should mark winner"), GameMode->HasWinner());
    TestFalse(TEXT("One round win should not end the match"), GameMode->ShouldEndMatchOnRoundWin(1));
    TestTrue(TEXT("Two round wins should end the match"), GameMode->ShouldEndMatchOnRoundWin(2));
    TestTrue(TEXT("Three round wins should also end the match"), GameMode->ShouldEndMatchOnRoundWin(3));
    TestFalse(TEXT("One player is not enough to leave lobby"), GameMode->IsReadyToStartMatch(1, 1));
    TestTrue(TEXT("Two connected players should be enough to leave lobby"), GameMode->IsReadyToStartMatch(2, 0));
    TestTrue(TEXT("Extra ready-state information should not block match start"), GameMode->IsReadyToStartMatch(2, 2));
    TestTrue(TEXT("Match-end countdown reaching zero should return to lobby"), GameMode->ShouldReturnToLobbyAfterMatchEnd(0));
    TestFalse(TEXT("Positive match-end countdown should keep result phase active"), GameMode->ShouldReturnToLobbyAfterMatchEnd(1));

    APvPArenaPlayerState* RoundWinner = NewObject<APvPArenaPlayerState>();
    TestNotNull(TEXT("Round winner player state should be created"), RoundWinner);
    if (!RoundWinner)
    {
        return false;
    }

    RoundWinner->AddRoundWin();
    TestEqual(
        TEXT("A single awarded round win should keep the match in the playing phase"),
        GameMode->ResolveMatchPhaseAfterRoundWin(RoundWinner->GetRoundWins()),
        EPvPAMatchPhase::Playing);
    TestTrue(
        TEXT("A single awarded round win should still continue to the next round"),
        GameMode->ShouldContinueToNextRoundAfterRoundWin(RoundWinner->GetRoundWins()));

    RoundWinner->AddRoundWin();
    TestEqual(
        TEXT("A second awarded round win should move the match into the match-end phase"),
        GameMode->ResolveMatchPhaseAfterRoundWin(RoundWinner->GetRoundWins()),
        EPvPAMatchPhase::MatchEnd);
    TestFalse(
        TEXT("A completed match should not continue into another round"),
        GameMode->ShouldContinueToNextRoundAfterRoundWin(RoundWinner->GetRoundWins()));

    RoundWinner->ResetMatchStats();
    TestEqual(TEXT("Resetting match stats should clear awarded round wins"), RoundWinner->GetRoundWins(), 0);

    TestTrue(
        TEXT("Sudden death should end round on next kill"),
        GameMode->ShouldEndRoundOnKill(EPvPARoundState::SuddenDeath, 0));
    TestFalse(
        TEXT("Playing round should not end below score limit"),
        GameMode->ShouldEndRoundOnKill(EPvPARoundState::Playing, 2));
    TestTrue(
        TEXT("Playing round should end at score limit"),
        GameMode->ShouldEndRoundOnKill(EPvPARoundState::Playing, 3));
    return true;
}
