#include "Misc/AutomationTest.h"
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
        TEXT("Crosshair should start hidden before ranged charge"),
        UPvPArenaHUDWidget::BuildRangedCrosshairVisibilityState(Character),
        ESlateVisibility::Collapsed);

    TestTrue(TEXT("Ranged charge should begin for crosshair visibility test"), Character->BeginRangedCharge(0.0f));
    TestTrue(TEXT("Charge begin should mark input as held"), Character->IsRangedChargeInputHeld());
    TestEqual(
        TEXT("Crosshair should show while ranged charge input is held"),
        UPvPArenaHUDWidget::BuildRangedCrosshairVisibilityState(Character),
        ESlateVisibility::Visible);

    TestFalse(TEXT("Early release should cancel ranged charge"), Character->ReleaseRangedCharge(0.1f));
    TestFalse(TEXT("Release should clear held input immediately"), Character->IsRangedChargeInputHeld());
    TestEqual(
        TEXT("Crosshair should hide immediately once ranged charge input is released"),
        UPvPArenaHUDWidget::BuildRangedCrosshairVisibilityState(Character),
        ESlateVisibility::Collapsed);

    return true;
}
