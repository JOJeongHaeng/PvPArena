#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameState.h"
#include "Game/PvPArenaPlayerState.h"
#include "UI/PvPArenaHUDWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHUDWidgetTeamScoreTest,
    "PvPArena.UI.HUDWidgetTeamScore",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDWidgetTeamScoreTest::RunTest(const FString& Parameters)
{
    APvPArenaGameState* GameState = NewObject<APvPArenaGameState>();
    TestNotNull(TEXT("GameState should be created"), GameState);

    if (!GameState)
    {
        return false;
    }

    APvPArenaPlayerState* LocalPlayer = NewObject<APvPArenaPlayerState>(GameState);
    APvPArenaPlayerState* AllyPlayer = NewObject<APvPArenaPlayerState>(GameState);
    APvPArenaPlayerState* EnemyPlayer = NewObject<APvPArenaPlayerState>(GameState);
    TestNotNull(TEXT("Local team player should be created"), LocalPlayer);
    TestNotNull(TEXT("Ally team player should be created"), AllyPlayer);
    TestNotNull(TEXT("Enemy team player should be created"), EnemyPlayer);

    if (!LocalPlayer || !AllyPlayer || !EnemyPlayer)
    {
        return false;
    }

    LocalPlayer->SetLobbyMatchMode(EPvPALobbyMatchMode::TeamVersus);
    LocalPlayer->SetLobbyTeam(EPvPALobbyTeam::Left);
    LocalPlayer->AddRoundWin();
    LocalPlayer->SetDisplayNickname(TEXT("BlueOne"));

    AllyPlayer->SetLobbyMatchMode(EPvPALobbyMatchMode::TeamVersus);
    AllyPlayer->SetLobbyTeam(EPvPALobbyTeam::Left);
    AllyPlayer->AddRoundWin();
    AllyPlayer->SetDisplayNickname(TEXT("BlueTwo"));

    EnemyPlayer->SetLobbyMatchMode(EPvPALobbyMatchMode::TeamVersus);
    EnemyPlayer->SetLobbyTeam(EPvPALobbyTeam::Right);
    EnemyPlayer->SetDisplayNickname(TEXT("RedOne"));

    GameState->SetRoundWinsToWin(3);
    GameState->SetRoundWinner(AllyPlayer);
    GameState->SetMatchWinner(AllyPlayer);
    GameState->PlayerArray = {LocalPlayer, AllyPlayer, EnemyPlayer};

    TestEqual(
        TEXT("Team round result should follow the winning team instead of the local player's death or kills"),
        UPvPArenaHUDWidget::BuildRoundResultLabel(LocalPlayer, GameState),
        FString(TEXT("Victory")));
    TestEqual(
        TEXT("Team match result should follow the winning team instead of the specific representative player"),
        UPvPArenaHUDWidget::BuildMatchResultLabel(LocalPlayer, GameState),
        FString(TEXT("Final Victory")));
    TestEqual(
        TEXT("Team match summary should show Blue and Red progress toward the target"),
        UPvPArenaHUDWidget::BuildMatchScoreSummary(LocalPlayer, GameState),
        FString(TEXT("Match Score: Blue 1 / 3 | Red 0 / 3")));

    GameState->SetRoundWinner(nullptr);
    GameState->SetMatchWinner(nullptr);
    EnemyPlayer->AddRoundWin();
    TestEqual(
        TEXT("Team round ties should render as a draw"),
        UPvPArenaHUDWidget::BuildRoundResultLabel(LocalPlayer, GameState),
        FString(TEXT("Draw")));
    return true;
}
