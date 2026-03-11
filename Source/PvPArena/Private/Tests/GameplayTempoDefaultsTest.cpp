#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameMode.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGameplayTempoDefaultsTest,
    "PvPArena.Match.GameplayTempoDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGameplayTempoDefaultsTest::RunTest(const FString& Parameters)
{
    APvPArenaGameMode* GameMode = NewObject<APvPArenaGameMode>();
    TestNotNull(TEXT("GameMode should be created"), GameMode);

    if (!GameMode)
    {
        return false;
    }

    TestEqual(TEXT("Iteration score limit should stay on the short test default"), GameMode->GetIterationScoreLimitDefault(), 3);
    TestEqual(TEXT("Iteration round duration should stay on the short test default"), GameMode->GetIterationRoundDurationSecondsDefault(), 60);
    TestEqual(TEXT("Final restore score limit target should remain documented"), GameMode->GetPlannedFinalScoreLimitDefault(), 5);
    TestEqual(TEXT("Final restore round duration target should remain documented"), GameMode->GetPlannedFinalRoundDurationSecondsDefault(), 180);
    TestEqual(TEXT("Respawn delay should match tuned default"), GameMode->GetRespawnDelaySeconds(), 2);
    TestEqual(TEXT("Round-end delay should match tuned default"), GameMode->GetRoundEndDelaySeconds(), 3);
    TestEqual(TEXT("Respawn invulnerability should match tuned default"), GameMode->GetRespawnInvulnerabilitySeconds(), 1.25f);
    TestFalse(
        TEXT("Playing round should not end below score limit"),
        GameMode->ShouldEndRoundOnKill(EPvPARoundState::Playing, 2));
    TestTrue(
        TEXT("Playing round should end at score limit"),
        GameMode->ShouldEndRoundOnKill(EPvPARoundState::Playing, 3));

    return true;
}
