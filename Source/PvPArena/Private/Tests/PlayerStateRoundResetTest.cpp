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
    PlayerState->AddRoundWin();
    PlayerState->SetReadyForLobbyStart(true);
    PlayerState->SetLobbyMatchMode(EPvPALobbyMatchMode::TeamVersus);
    PlayerState->SetLobbyTeam(EPvPALobbyTeam::Left);
    TestEqual(TEXT("Round kills should increment"), PlayerState->GetRoundKills(), 1);
    TestEqual(TEXT("Round deaths should increment"), PlayerState->GetRoundDeaths(), 1);
    TestEqual(TEXT("Match kills should increment"), PlayerState->GetMatchKills(), 1);
    TestEqual(TEXT("Match deaths should increment"), PlayerState->GetMatchDeaths(), 1);
    TestEqual(TEXT("Round wins should increment"), PlayerState->GetRoundWins(), 1);
    TestTrue(TEXT("Ready state should update"), PlayerState->IsReadyForLobbyStart());
    TestEqual(TEXT("Lobby match mode should update"), PlayerState->GetLobbyMatchMode(), EPvPALobbyMatchMode::TeamVersus);
    TestEqual(TEXT("Lobby team should update"), PlayerState->GetLobbyTeam(), EPvPALobbyTeam::Left);

    PlayerState->ResetRoundStats();
    TestEqual(TEXT("Round kills should reset"), PlayerState->GetRoundKills(), 0);
    TestEqual(TEXT("Round deaths should reset"), PlayerState->GetRoundDeaths(), 0);
    TestEqual(TEXT("Match kills should persist across round reset"), PlayerState->GetMatchKills(), 1);
    TestEqual(TEXT("Match deaths should persist across round reset"), PlayerState->GetMatchDeaths(), 1);
    TestEqual(TEXT("Round wins should persist across round reset"), PlayerState->GetRoundWins(), 1);
    TestEqual(TEXT("Lobby match mode should persist across round reset"), PlayerState->GetLobbyMatchMode(), EPvPALobbyMatchMode::TeamVersus);
    TestEqual(TEXT("Lobby team should persist across round reset"), PlayerState->GetLobbyTeam(), EPvPALobbyTeam::Left);

    PlayerState->ResetLobbyStateForModeChange(EPvPALobbyMatchMode::FreeForAll);
    TestFalse(TEXT("Ready state should clear when lobby mode changes"), PlayerState->IsReadyForLobbyStart());
    TestEqual(TEXT("Mode change should update the stored lobby mode"), PlayerState->GetLobbyMatchMode(), EPvPALobbyMatchMode::FreeForAll);
    TestEqual(TEXT("Free-for-all mode should ignore prior team selection"), PlayerState->GetLobbyTeam(), EPvPALobbyTeam::None);

    PlayerState->SetReadyForLobbyStart(true);
    PlayerState->SetLobbyTeam(EPvPALobbyTeam::Right);
    PlayerState->ResetLobbyStateForModeChange(EPvPALobbyMatchMode::TeamVersus);
    TestFalse(TEXT("Ready state should clear again on team-mode changes"), PlayerState->IsReadyForLobbyStart());
    TestEqual(TEXT("Team mode should be stored after reset"), PlayerState->GetLobbyMatchMode(), EPvPALobbyMatchMode::TeamVersus);
    TestEqual(TEXT("Team-mode reset should force players to choose a team again"), PlayerState->GetLobbyTeam(), EPvPALobbyTeam::None);

    PlayerState->SetDisplayNickname(TEXT("ArenaHero"));
    PlayerState->SetLobbyMatchMode(EPvPALobbyMatchMode::TeamVersus);
    PlayerState->SetLobbyTeam(EPvPALobbyTeam::Left);
    PlayerState->ResetMatchStats();
    TestEqual(TEXT("Match kills should reset across match reset"), PlayerState->GetMatchKills(), 0);
    TestEqual(TEXT("Match deaths should reset across match reset"), PlayerState->GetMatchDeaths(), 0);
    TestEqual(TEXT("Round wins should reset across match reset"), PlayerState->GetRoundWins(), 0);
    TestFalse(TEXT("Ready state should reset across match reset"), PlayerState->IsReadyForLobbyStart());
    TestEqual(TEXT("Display nickname should survive match reset"), PlayerState->GetDisplayNickname(), FString(TEXT("ArenaHero")));
    TestEqual(TEXT("Match reset should preserve the selected lobby mode for the next in-game round"), PlayerState->GetLobbyMatchMode(), EPvPALobbyMatchMode::TeamVersus);
    TestEqual(TEXT("Match reset should preserve the selected lobby team so teammates still identify correctly in game"), PlayerState->GetLobbyTeam(), EPvPALobbyTeam::Left);
    return true;
}
