#include "Misc/AutomationTest.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "UI/PvPArenaHUDWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHUDWidgetLayoutTest,
    "PvPArena.UI.HUDWidgetLayout",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDWidgetLayoutTest::RunTest(const FString& Parameters)
{
    UPvPArenaHUDWidget* HUDWidget = NewObject<UPvPArenaHUDWidget>();
    TestNotNull(TEXT("HUD widget should be created"), HUDWidget);

    if (!HUDWidget)
    {
        return false;
    }

    HUDWidget->TakeWidget();

    UWidgetTree* WidgetTree = HUDWidget->WidgetTree;
    TestNotNull(TEXT("HUD widget should have a widget tree"), WidgetTree);

    if (!WidgetTree)
    {
        return false;
    }

    UOverlay* RootOverlay = Cast<UOverlay>(WidgetTree->RootWidget);
    TestNotNull(TEXT("HUD widget should build an overlay root"), RootOverlay);
    TestEqual(TEXT("HUD root should contain status and announcement layers"), RootOverlay ? RootOverlay->GetChildrenCount() : 0, 2);

    UVerticalBox* StatusBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("StatusBox")));
    UVerticalBox* AnnouncementBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("AnnouncementBox")));
    TestNotNull(TEXT("StatusBox should exist"), StatusBox);
    TestNotNull(TEXT("AnnouncementBox should exist"), AnnouncementBox);
    TestEqual(TEXT("StatusBox should contain the six always-on HUD rows"), StatusBox ? StatusBox->GetChildrenCount() : 0, 6);
    TestEqual(TEXT("AnnouncementBox should contain the two round-end HUD rows"), AnnouncementBox ? AnnouncementBox->GetChildrenCount() : 0, 2);

    UTextBlock* HealthText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("HealthText")));

    TestNotNull(TEXT("HealthBar should exist"), WidgetTree->FindWidget(TEXT("HealthBar")));
    TestNotNull(TEXT("HealthText should exist"), HealthText);
    TestNotNull(TEXT("RoundScoreText should exist"), WidgetTree->FindWidget(TEXT("RoundScoreText")));
    TestNotNull(TEXT("MatchScoreText should exist"), WidgetTree->FindWidget(TEXT("MatchScoreText")));
    TestNotNull(TEXT("TimerText should exist"), WidgetTree->FindWidget(TEXT("TimerText")));
    TestNotNull(TEXT("RoundStateText should exist"), WidgetTree->FindWidget(TEXT("RoundStateText")));
    UTextBlock* ResultText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("ResultText")));
    UTextBlock* NextRoundText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("NextRoundText")));

    TestNotNull(TEXT("ResultText should exist"), ResultText);
    TestNotNull(TEXT("NextRoundText should exist"), NextRoundText);
    TestEqual(TEXT("ResultText should stay hidden until round end"),
        ResultText ? ResultText->GetVisibility() : ESlateVisibility::Visible,
        ESlateVisibility::Collapsed);
    TestEqual(TEXT("NextRoundText should stay hidden until round end"),
        NextRoundText ? NextRoundText->GetVisibility() : ESlateVisibility::Visible,
        ESlateVisibility::Collapsed);
    TestTrue(TEXT("ResultText should use a larger font than default status rows"),
        ResultText && ResultText->GetFont().Size >= 30);
    TestTrue(TEXT("NextRoundText should use a larger font than default status rows"),
        NextRoundText && NextRoundText->GetFont().Size >= 22);
    return true;
}
