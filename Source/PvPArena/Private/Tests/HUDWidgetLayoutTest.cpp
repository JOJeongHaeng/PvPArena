#include "Misc/AutomationTest.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
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

    UBorder* StatusPanel = Cast<UBorder>(WidgetTree->FindWidget(TEXT("StatusPanel")));
    UBorder* AnnouncementPanel = Cast<UBorder>(WidgetTree->FindWidget(TEXT("AnnouncementPanel")));
    UVerticalBox* StatusBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("StatusBox")));
    UVerticalBox* AnnouncementBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("AnnouncementBox")));
    TestNotNull(TEXT("StatusPanel should exist"), StatusPanel);
    TestNotNull(TEXT("AnnouncementPanel should exist"), AnnouncementPanel);
    TestNotNull(TEXT("StatusBox should exist"), StatusBox);
    TestNotNull(TEXT("AnnouncementBox should exist"), AnnouncementBox);
    TestEqual(TEXT("StatusBox should contain the six always-on HUD rows"), StatusBox ? StatusBox->GetChildrenCount() : 0, 6);
    TestEqual(TEXT("AnnouncementBox should contain the two round-end HUD rows"), AnnouncementBox ? AnnouncementBox->GetChildrenCount() : 0, 2);

    USizeBox* HealthBarSizeBox = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("HealthBarSizeBox")));
    UProgressBar* HealthBar = Cast<UProgressBar>(WidgetTree->FindWidget(TEXT("HealthBar")));
    UTextBlock* HealthText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("HealthText")));
    UTextBlock* RoundScoreText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("RoundScoreText")));
    UTextBlock* MatchScoreText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("MatchScoreText")));
    UTextBlock* TimerText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TimerText")));
    UTextBlock* RoundStateText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("RoundStateText")));

    TestNotNull(TEXT("HealthBarSizeBox should exist"), HealthBarSizeBox);
    TestNotNull(TEXT("HealthBar should exist"), HealthBar);
    TestNotNull(TEXT("HealthText should exist"), HealthText);
    TestNotNull(TEXT("RoundScoreText should exist"), RoundScoreText);
    TestNotNull(TEXT("MatchScoreText should exist"), MatchScoreText);
    TestNotNull(TEXT("TimerText should exist"), TimerText);
    TestNotNull(TEXT("RoundStateText should exist"), RoundStateText);
    UTextBlock* ResultText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("ResultText")));
    UTextBlock* NextRoundText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("NextRoundText")));

    TestTrue(TEXT("Health bar should use a thicker readability-first height"),
        HealthBarSizeBox && HealthBarSizeBox->GetHeightOverride() >= 20.0f);
    TestTrue(TEXT("Health text should be larger than default small body text"),
        HealthText && HealthText->GetFont().Size >= 18);
    TestTrue(TEXT("Round score text should be larger than default small body text"),
        RoundScoreText && RoundScoreText->GetFont().Size >= 18);
    TestTrue(TEXT("Match score text should be larger than default small body text"),
        MatchScoreText && MatchScoreText->GetFont().Size >= 18);
    TestTrue(TEXT("Timer text should be larger than default small body text"),
        TimerText && TimerText->GetFont().Size >= 18);
    TestTrue(TEXT("Round state text should be larger than default small body text"),
        RoundStateText && RoundStateText->GetFont().Size >= 18);

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
    TestTrue(TEXT("ResultText should remain larger than the status text hierarchy"),
        ResultText && HealthText && ResultText->GetFont().Size > HealthText->GetFont().Size);
    TestTrue(TEXT("Status panel should use a visible dark backing"),
        StatusPanel && StatusPanel->GetBrushColor().A >= 0.7f);
    TestTrue(TEXT("Announcement panel should use a visible dark backing"),
        AnnouncementPanel && AnnouncementPanel->GetBrushColor().A >= 0.8f);
    return true;
}
