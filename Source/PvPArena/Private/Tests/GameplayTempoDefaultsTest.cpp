#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameMode.h"
#include "Player/PvPArenaCharacter.h"
#include "Player/PvPArenaSpectatorPawn.h"

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

    TestEqual(TEXT("Iteration score limit should match the free-for-all early-win target"), GameMode->GetIterationScoreLimitDefault(), 5);
    TestEqual(TEXT("Iteration round wins-to-win should require three wins"), GameMode->GetIterationRoundWinsToWinDefault(), 3);
    TestEqual(TEXT("Lobby countdown should be three seconds"), GameMode->GetLobbyCountdownSeconds(), 3);
    TestEqual(TEXT("Iteration round duration should match the free-for-all single-match timer"), GameMode->GetIterationRoundDurationSecondsDefault(), 180);
    TestEqual(TEXT("Final restore score limit target should remain documented"), GameMode->GetPlannedFinalScoreLimitDefault(), 5);
    TestEqual(TEXT("Final restore round duration target should remain documented"), GameMode->GetPlannedFinalRoundDurationSecondsDefault(), 180);
    TestEqual(TEXT("Respawn delay should match tuned default"), GameMode->GetRespawnDelaySeconds(), 2);
    TestEqual(TEXT("Round-end delay should match tuned default"), GameMode->GetRoundEndDelaySeconds(), 3);
    TestEqual(TEXT("Respawn invulnerability should match tuned default"), GameMode->GetRespawnInvulnerabilitySeconds(), 1.25f);
    TestEqual(TEXT("Game mode should use the custom free-fly spectator pawn"), GameMode->GetConfiguredSpectatorClass(), TSubclassOf<ASpectatorPawn>(APvPArenaSpectatorPawn::StaticClass()));
    TestFalse(
        TEXT("Playing round should not end below score limit"),
        GameMode->ShouldEndRoundOnKill(EPvPARoundState::Playing, 4));
    TestTrue(
        TEXT("Playing round should end at score limit"),
        GameMode->ShouldEndRoundOnKill(EPvPARoundState::Playing, 5));

    APvPArenaCharacter* Character = NewObject<APvPArenaCharacter>();
    TestNotNull(TEXT("Character should be created"), Character);
    if (!Character)
    {
        return false;
    }

    TestEqual(TEXT("Sprint duration should be one point five seconds"), Character->GetSprintDurationSeconds(), 1.5f);
    TestEqual(TEXT("Sprint speed multiplier should be one point five"), Character->GetSprintSpeedMultiplier(), 1.5f);
    TestEqual(TEXT("Sprint recharge rate should be slightly slower than drain"), Character->GetSprintRechargeRate(), 0.75f);

    return true;
}
