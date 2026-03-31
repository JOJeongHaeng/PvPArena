#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameMode.h"
#include "Game/PvPArenaPlayerState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPlayerStateDisplayNameTest,
    "PvPArena.Match.PlayerStateDisplayName",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerStateDisplayNameTest::RunTest(const FString& Parameters)
{
    TestEqual(
        TEXT("Whitespace-only nickname should normalize to empty"),
        APvPArenaPlayerState::BuildNormalizedDisplayNickname(TEXT("   ")),
        FString());
    TestEqual(
        TEXT("Nickname normalization should trim outer whitespace"),
        APvPArenaPlayerState::BuildNormalizedDisplayNickname(TEXT("  Arena Hero  ")),
        FString(TEXT("Arena Hero")));
    TestEqual(
        TEXT("First fallback nickname should resolve to Player1"),
        APvPArenaGameMode::BuildDefaultDisplayNickname(1),
        FString(TEXT("Player1")));
    TestEqual(
        TEXT("Sixth fallback nickname should resolve to Player6"),
        APvPArenaGameMode::BuildDefaultDisplayNickname(6),
        FString(TEXT("Player6")));

    APvPArenaPlayerState* PlayerState = NewObject<APvPArenaPlayerState>();
    TestNotNull(TEXT("PlayerState should be created"), PlayerState);

    if (!PlayerState)
    {
        return false;
    }

    PlayerState->SetDisplayNickname(TEXT("Player3"));
    TestEqual(TEXT("Stored display nickname should round-trip"), PlayerState->GetDisplayNickname(), FString(TEXT("Player3")));

    return true;
}
