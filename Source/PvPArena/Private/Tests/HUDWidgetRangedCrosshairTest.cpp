#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameState.h"
#include "Player/PvPArenaCharacter.h"
#include "UI/PvPArenaHUDWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHUDWidgetRangedCrosshairTest,
    "PvPArena.UI.HUDWidgetRangedCrosshair",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDWidgetRangedCrosshairTest::RunTest(const FString& Parameters)
{
    APvPArenaCharacter* Character = NewObject<APvPArenaCharacter>();
    TestNotNull(TEXT("Character should be created"), Character);

    if (!Character)
    {
        return false;
    }

    TestEqual(
        TEXT("Crosshair should stay hidden without match state"),
        UPvPArenaHUDWidget::BuildRangedCrosshairVisibilityState(Character),
        ESlateVisibility::Collapsed);
    TestEqual(
        TEXT("Crosshair should not offset while hidden without match state"),
        UPvPArenaHUDWidget::BuildRangedCrosshairVerticalOffsetState(Character),
        0.0f);

    APvPArenaGameState* GameState = NewObject<APvPArenaGameState>();
    TestNotNull(TEXT("GameState should be created"), GameState);

    if (!GameState)
    {
        return false;
    }

    GameState->SetMatchPhase(EPvPAMatchPhase::Playing);
    TestEqual(
        TEXT("Crosshair should show throughout active match play"),
        UPvPArenaHUDWidget::BuildRangedCrosshairVisibilityState(Character, GameState),
        ESlateVisibility::Visible);
    TestEqual(
        TEXT("Crosshair should shift upward during default play"),
        UPvPArenaHUDWidget::BuildRangedCrosshairVerticalOffsetState(Character, GameState),
        -24.0f);

    const bool bChargeStarted = Character->BeginRangedCharge(0.0f);
    TestTrue(TEXT("Character should enter ranged charge for aim-state HUD checks"), bChargeStarted);
    TestEqual(
        TEXT("Crosshair should return to the center while right-click aim is held"),
        UPvPArenaHUDWidget::BuildRangedCrosshairVerticalOffsetState(Character, GameState),
        0.0f);

    GameState->SetMatchPhase(EPvPAMatchPhase::Lobby);
    TestEqual(
        TEXT("Crosshair should stay hidden in the lobby"),
        UPvPArenaHUDWidget::BuildRangedCrosshairVisibilityState(Character, GameState),
        ESlateVisibility::Collapsed);
    TestEqual(
        TEXT("Crosshair should not keep an offset when hidden in the lobby"),
        UPvPArenaHUDWidget::BuildRangedCrosshairVerticalOffsetState(Character, GameState),
        0.0f);

    GameState->SetMatchPhase(EPvPAMatchPhase::MatchEnd);
    TestEqual(
        TEXT("Crosshair should hide again once the match ends"),
        UPvPArenaHUDWidget::BuildRangedCrosshairVisibilityState(Character, GameState),
        ESlateVisibility::Collapsed);
    TestEqual(
        TEXT("Crosshair should reset its visual offset once the match ends"),
        UPvPArenaHUDWidget::BuildRangedCrosshairVerticalOffsetState(Character, GameState),
        0.0f);

    return true;
}
