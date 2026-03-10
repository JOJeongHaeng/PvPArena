#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameMode.h"

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
