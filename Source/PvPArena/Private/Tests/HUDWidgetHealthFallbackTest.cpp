#include "Misc/AutomationTest.h"
#include "UI/PvPArenaHUDWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHUDWidgetHealthFallbackTest,
    "PvPArena.UI.HUDWidgetHealthFallback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDWidgetHealthFallbackTest::RunTest(const FString& Parameters)
{
    float HealthPercent = 1.0f;
    FString HealthLabel;

    UPvPArenaHUDWidget::BuildHealthDisplayState(nullptr, HealthPercent, HealthLabel);

    TestEqual(TEXT("Health percent should drop to zero when no pawn is possessed"), HealthPercent, 0.0f);
    TestEqual(TEXT("Health text should indicate no active pawn"), HealthLabel, FString(TEXT("HP: --")));
    return true;
}
