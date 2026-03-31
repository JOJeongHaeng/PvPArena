#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameState.h"
#include "Game/PvPArenaPlayerState.h"
#include "UI/PvPArenaHUDWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHUDWidgetLobbyPlayerListTest,
    "PvPArena.UI.HUDWidgetLobbyPlayerList",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDWidgetLobbyPlayerListTest::RunTest(const FString& Parameters)
{
    APvPArenaGameState* GameState = NewObject<APvPArenaGameState>();
    TestNotNull(TEXT("GameState should be created"), GameState);

    if (!GameState)
    {
        return false;
    }

    APvPArenaPlayerState* PlayerOne = NewObject<APvPArenaPlayerState>(GameState);
    APvPArenaPlayerState* PlayerTwo = NewObject<APvPArenaPlayerState>(GameState);
    APvPArenaPlayerState* PlayerThree = NewObject<APvPArenaPlayerState>(GameState);
    PlayerOne->SetDisplayNickname(TEXT("Player1"));
    PlayerTwo->SetDisplayNickname(TEXT("ArenaHero"));
    PlayerThree->SetDisplayNickname(TEXT("Player3"));

    GameState->PlayerArray = {PlayerOne, PlayerTwo, PlayerThree};

    const TArray<FString> ParticipantLabels = UPvPArenaHUDWidget::BuildLobbyParticipantLabels(GameState);
    TestEqual(TEXT("Participant count should match player array"), ParticipantLabels.Num(), 3);
    TestEqual(TEXT("First participant should show fallback nickname"), ParticipantLabels[0], FString(TEXT("Player1")));
    TestEqual(TEXT("Second participant should show the custom nickname"), ParticipantLabels[1], FString(TEXT("ArenaHero")));
    TestEqual(
        TEXT("Empty nickname draft should resolve to Player1 placeholder"),
        UPvPArenaHUDWidget::BuildLobbyNicknameTextBoxValue(TEXT(""), TEXT("Player1")),
        FString(TEXT("Player1")));

    return true;
}
