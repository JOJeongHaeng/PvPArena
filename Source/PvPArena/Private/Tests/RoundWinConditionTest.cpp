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
        TEXT("Free-for-all timeout ties should end the round so tied leaders can both score"),
        GameMode->ResolveRoundTimeout(3, 3),
        EPvPARoundState::RoundEnd);
    TestTrue(TEXT("Timeout resolution should still mark the round as decided"), GameMode->HasWinner());
    TestEqual(
        TEXT("Explicit free-for-all tie helper should also end the round"),
        GameMode->ResolveFreeForAllRoundTimeoutState(true),
        EPvPARoundState::RoundEnd);
    TestEqual(
        TEXT("Explicit free-for-all non-tie helper should end the round"),
        GameMode->ResolveFreeForAllRoundTimeoutState(false),
        EPvPARoundState::RoundEnd);
    TestEqual(
        TEXT("Team-versus timeout should end the round when left team has more survivors"),
        GameMode->ResolveTeamVersusRoundTimeoutState(2, 1),
        EPvPARoundState::RoundEnd);
    TestEqual(
        TEXT("Team-versus timeout should also end the round on a survivor tie so both teams can score"),
        GameMode->ResolveTeamVersusRoundTimeoutState(1, 1),
        EPvPARoundState::RoundEnd);

    TestEqual(
        TEXT("Non-tie timeout should end round"),
        GameMode->ResolveRoundTimeout(4, 2),
        EPvPARoundState::RoundEnd);
    TestTrue(TEXT("Non-tie timeout should mark winner"), GameMode->HasWinner());
    TestFalse(TEXT("One round win should not end the match"), GameMode->ShouldEndMatchOnRoundWin(1));
    TestFalse(TEXT("Two untied round wins should still continue toward the third win"), GameMode->ShouldEndMatchOnRoundWin(2));
    TestFalse(
        TEXT("Tied teams at two wins should force an extra round"),
        GameMode->ShouldEndMatchOnRoundWinState(2, true));
    TestTrue(
        TEXT("An untied leader at the target round wins should still end the match"),
        GameMode->ShouldEndMatchOnRoundWinState(3, false));
    TestTrue(TEXT("Three round wins should also end the match"), GameMode->ShouldEndMatchOnRoundWin(3));
    TestEqual(TEXT("Lobby should cap out at six players"), GameMode->GetMaximumLobbyPlayers(), 6);
    TestFalse(
        TEXT("One player is not enough to leave a free-for-all lobby"),
        GameMode->IsReadyToStartMatch(EPvPALobbyMatchMode::FreeForAll, 1, 0, 0));
    TestTrue(
        TEXT("Two connected players should be enough to leave a free-for-all lobby"),
        GameMode->IsReadyToStartMatch(EPvPALobbyMatchMode::FreeForAll, 2, 0, 0));
    TestTrue(
        TEXT("Ready-state information should not block free-for-all match start"),
        GameMode->IsReadyToStartMatch(EPvPALobbyMatchMode::FreeForAll, 6, 0, 0));
    TestFalse(
        TEXT("Team mode should not start without a player on the left team"),
        GameMode->IsReadyToStartMatch(EPvPALobbyMatchMode::TeamVersus, 2, 0, 2));
    TestFalse(
        TEXT("Team mode should not start without a player on the right team"),
        GameMode->IsReadyToStartMatch(EPvPALobbyMatchMode::TeamVersus, 2, 2, 0));
    TestTrue(
        TEXT("Team mode should allow asymmetric rosters as long as both teams are represented"),
        GameMode->IsReadyToStartMatch(EPvPALobbyMatchMode::TeamVersus, 5, 1, 4));
    TestFalse(TEXT("A non-host client should not be able to start the lobby match"), GameMode->CanLobbyHostStartMatch(false, 2));
    TestFalse(TEXT("The host still needs enough players before starting"), GameMode->CanLobbyHostStartMatch(true, 1));
    TestTrue(TEXT("The host should be able to start once enough players are connected"), GameMode->CanLobbyHostStartMatch(true, 2));
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
        TEXT("A single awarded round win should keep the team-versus match in the playing phase"),
        GameMode->ResolveMatchPhaseAfterRoundWin(RoundWinner->GetRoundWins()),
        EPvPAMatchPhase::Playing);
    TestTrue(
        TEXT("A single awarded round win should still continue to the next round"),
        GameMode->ShouldContinueToNextRoundAfterRoundWin(RoundWinner->GetRoundWins()));

    RoundWinner->AddRoundWin();
    TestEqual(
        TEXT("A second awarded round win should still keep the team-versus match in the playing phase"),
        GameMode->ResolveMatchPhaseAfterRoundWin(RoundWinner->GetRoundWins()),
        EPvPAMatchPhase::Playing);
    TestTrue(
        TEXT("Two wins should still continue to the next round under first-to-three"),
        GameMode->ShouldContinueToNextRoundAfterRoundWin(RoundWinner->GetRoundWins()));

    RoundWinner->AddRoundWin();
    TestEqual(
        TEXT("A third awarded round win should move the team-versus match into the match-end phase"),
        GameMode->ResolveMatchPhaseAfterRoundWin(RoundWinner->GetRoundWins()),
        EPvPAMatchPhase::MatchEnd);
    TestFalse(
        TEXT("A completed team-versus match should not continue into another round"),
        GameMode->ShouldContinueToNextRoundAfterRoundWin(RoundWinner->GetRoundWins()));
    TestEqual(
        TEXT("Tied teams at the win target should stay in the playing phase for an extra round"),
        GameMode->ResolveMatchPhaseAfterRoundWinState(3, true),
        EPvPAMatchPhase::Playing);

    RoundWinner->ResetMatchStats();
    TestEqual(TEXT("Resetting match stats should clear awarded round wins"), RoundWinner->GetRoundWins(), 0);

    APvPArenaPlayerState* LobbyPlayer = NewObject<APvPArenaPlayerState>();
    TestNotNull(TEXT("Lobby player state should be created"), LobbyPlayer);
    if (!LobbyPlayer)
    {
        return false;
    }

    LobbyPlayer->SetReadyForLobbyStart(true);
    LobbyPlayer->SetLobbyMatchMode(EPvPALobbyMatchMode::TeamVersus);
    LobbyPlayer->SetLobbyTeam(EPvPALobbyTeam::Left);
    GameMode->ApplyLobbyModeChangeToPlayerState(LobbyPlayer, EPvPALobbyMatchMode::FreeForAll);
    TestFalse(TEXT("Mode change should clear ready state"), LobbyPlayer->IsReadyForLobbyStart());
    TestEqual(TEXT("Mode change should update the player's stored mode"), LobbyPlayer->GetLobbyMatchMode(), EPvPALobbyMatchMode::FreeForAll);
    TestEqual(TEXT("Free-for-all mode should clear team selection"), LobbyPlayer->GetLobbyTeam(), EPvPALobbyTeam::None);

    LobbyPlayer->SetReadyForLobbyStart(true);
    LobbyPlayer->SetLobbyTeam(EPvPALobbyTeam::Right);
    GameMode->ApplyLobbyModeChangeToPlayerState(LobbyPlayer, EPvPALobbyMatchMode::TeamVersus);
    TestFalse(TEXT("Switching into team mode should also clear ready state"), LobbyPlayer->IsReadyForLobbyStart());
    TestEqual(TEXT("Switching into team mode should force team reselection"), LobbyPlayer->GetLobbyTeam(), EPvPALobbyTeam::None);

    TestTrue(
        TEXT("Sudden death should end round on next kill"),
        GameMode->ShouldEndRoundOnKill(EPvPARoundState::SuddenDeath, 0));
    TestFalse(
        TEXT("Playing round should not end below score limit"),
        GameMode->ShouldEndRoundOnKill(EPvPARoundState::Playing, 4));
    TestTrue(
        TEXT("Playing round should end at score limit"),
        GameMode->ShouldEndRoundOnKill(EPvPARoundState::Playing, 5));
    return true;
}
