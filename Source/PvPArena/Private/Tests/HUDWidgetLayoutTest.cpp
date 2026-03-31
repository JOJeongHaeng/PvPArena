#include "Misc/AutomationTest.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
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
    TestEqual(TEXT("HUD root should contain gauge, countdown, info, lobby, lobby controls, result, announcement, crosshair, and settings layers"), RootOverlay ? RootOverlay->GetChildrenCount() : 0, 9);

    UBorder* StatusPanel = Cast<UBorder>(WidgetTree->FindWidget(TEXT("StatusPanel")));
    UBorder* CountdownPanel = Cast<UBorder>(WidgetTree->FindWidget(TEXT("CountdownPanel")));
    UBorder* InfoPanel = Cast<UBorder>(WidgetTree->FindWidget(TEXT("InfoPanel")));
    UBorder* AnnouncementPanel = Cast<UBorder>(WidgetTree->FindWidget(TEXT("AnnouncementPanel")));
    UBorder* LobbyPanel = Cast<UBorder>(WidgetTree->FindWidget(TEXT("LobbyPanel")));
    UBorder* LobbyControlsPanel = Cast<UBorder>(WidgetTree->FindWidget(TEXT("LobbyControlsPanel")));
    UBorder* SettingsPanel = Cast<UBorder>(WidgetTree->FindWidget(TEXT("SettingsPanel")));
    UBorder* LobbyKeyboardCard = Cast<UBorder>(WidgetTree->FindWidget(TEXT("LobbyKeyboardCard")));
    UBorder* LobbyMouseCard = Cast<UBorder>(WidgetTree->FindWidget(TEXT("LobbyMouseCard")));
    UHorizontalBox* StatusCardsBox = Cast<UHorizontalBox>(WidgetTree->FindWidget(TEXT("StatusCardsBox")));
    UBorder* HealthCard = Cast<UBorder>(WidgetTree->FindWidget(TEXT("HealthCard")));
    UBorder* SprintCard = Cast<UBorder>(WidgetTree->FindWidget(TEXT("SprintCard")));
    UBorder* RangedCard = Cast<UBorder>(WidgetTree->FindWidget(TEXT("RangedCard")));
    UVerticalBox* HealthCardBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("HealthCardBox")));
    UVerticalBox* SprintCardBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("SprintCardBox")));
    UVerticalBox* RangedCardBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("RangedCardBox")));
    UVerticalBox* InfoBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("InfoBox")));
    UVerticalBox* AnnouncementBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("AnnouncementBox")));
    UVerticalBox* LobbyBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("LobbyBox")));
    UVerticalBox* LobbyControlsBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("LobbyControlsBox")));
    UVerticalBox* LobbyPlayerListBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("LobbyPlayerListBox")));
    UHorizontalBox* LobbyControlsCardsBox = Cast<UHorizontalBox>(WidgetTree->FindWidget(TEXT("LobbyControlsCardsBox")));
    UVerticalBox* LobbyKeyboardCardBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("LobbyKeyboardCardBox")));
    UVerticalBox* LobbyMouseCardBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("LobbyMouseCardBox")));
    TestNotNull(TEXT("StatusPanel should exist"), StatusPanel);
    TestNotNull(TEXT("CountdownPanel should exist"), CountdownPanel);
    TestNotNull(TEXT("InfoPanel should exist"), InfoPanel);
    TestNotNull(TEXT("AnnouncementPanel should exist"), AnnouncementPanel);
    TestNotNull(TEXT("LobbyPanel should exist"), LobbyPanel);
    TestNotNull(TEXT("LobbyControlsPanel should exist"), LobbyControlsPanel);
    TestNotNull(TEXT("SettingsPanel should exist"), SettingsPanel);
    TestNotNull(TEXT("Lobby keyboard card should exist"), LobbyKeyboardCard);
    TestNotNull(TEXT("Lobby mouse card should exist"), LobbyMouseCard);
    TestNotNull(TEXT("StatusCardsBox should exist"), StatusCardsBox);
    TestNotNull(TEXT("HealthCard should exist"), HealthCard);
    TestNotNull(TEXT("SprintCard should exist"), SprintCard);
    TestNotNull(TEXT("RangedCard should exist"), RangedCard);
    TestNotNull(TEXT("HealthCardBox should exist"), HealthCardBox);
    TestNotNull(TEXT("SprintCardBox should exist"), SprintCardBox);
    TestNotNull(TEXT("RangedCardBox should exist"), RangedCardBox);
    TestNotNull(TEXT("InfoBox should exist"), InfoBox);
    TestNotNull(TEXT("AnnouncementBox should exist"), AnnouncementBox);
    TestNotNull(TEXT("LobbyBox should exist"), LobbyBox);
    TestNotNull(TEXT("LobbyControlsBox should exist"), LobbyControlsBox);
    TestNotNull(TEXT("Lobby player list box should exist"), LobbyPlayerListBox);
    TestNotNull(TEXT("Lobby controls cards box should exist"), LobbyControlsCardsBox);
    TestNotNull(TEXT("Lobby keyboard card box should exist"), LobbyKeyboardCardBox);
    TestNotNull(TEXT("Lobby mouse card box should exist"), LobbyMouseCardBox);
    TestEqual(TEXT("Status card row should contain health, sprint, and ranged cards"), StatusCardsBox ? StatusCardsBox->GetChildrenCount() : 0, 3);
    TestEqual(TEXT("Health card should contain its bar and label"), HealthCardBox ? HealthCardBox->GetChildrenCount() : 0, 2);
    TestEqual(TEXT("Sprint card should contain its bar and label"), SprintCardBox ? SprintCardBox->GetChildrenCount() : 0, 2);
    TestEqual(TEXT("Ranged card should contain its bar and label"), RangedCardBox ? RangedCardBox->GetChildrenCount() : 0, 2);
    TestEqual(TEXT("InfoBox should contain the three score and round info rows"), InfoBox ? InfoBox->GetChildrenCount() : 0, 3);
    TestEqual(TEXT("AnnouncementBox should contain the two round-end HUD rows"), AnnouncementBox ? AnnouncementBox->GetChildrenCount() : 0, 2);
    TestEqual(TEXT("Lobby box should contain title, status, connection state, nickname entry, player list, IP entry, host/join actions, and ready button"), LobbyBox ? LobbyBox->GetChildrenCount() : 0, 9);
    TestTrue(TEXT("Lobby player list should start empty and be populated from player states"), LobbyPlayerListBox && LobbyPlayerListBox->GetChildrenCount() == 0);
    TestEqual(TEXT("Lobby controls box should contain only the controls title and controls cards row"), LobbyControlsBox ? LobbyControlsBox->GetChildrenCount() : 0, 2);
    TestEqual(TEXT("Lobby controls cards row should contain keyboard and mouse cards"), LobbyControlsCardsBox ? LobbyControlsCardsBox->GetChildrenCount() : 0, 2);
    TestEqual(TEXT("Lobby keyboard card should contain title, move, and sprint rows"), LobbyKeyboardCardBox ? LobbyKeyboardCardBox->GetChildrenCount() : 0, 3);
    TestEqual(TEXT("Lobby mouse card should contain title, melee, and ranged rows"), LobbyMouseCardBox ? LobbyMouseCardBox->GetChildrenCount() : 0, 3);

    USizeBox* HealthBarSizeBox = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("HealthBarSizeBox")));
    UProgressBar* HealthBar = Cast<UProgressBar>(WidgetTree->FindWidget(TEXT("HealthBar")));
    USizeBox* RangedCooldownBarSizeBox = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("RangedCooldownBarSizeBox")));
    UProgressBar* RangedCooldownBar = Cast<UProgressBar>(WidgetTree->FindWidget(TEXT("RangedCooldownBar")));
    UTextBlock* HealthText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("HealthText")));
    USizeBox* SprintBarSizeBox = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("SprintBarSizeBox")));
    UProgressBar* SprintBar = Cast<UProgressBar>(WidgetTree->FindWidget(TEXT("SprintBar")));
    UTextBlock* SprintText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("SprintText")));
    UTextBlock* RangedCooldownText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("RangedCooldownText")));
    UTextBlock* RoundScoreText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("RoundScoreText")));
    UTextBlock* MatchScoreText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("MatchScoreText")));
    UTextBlock* TimerText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TimerText")));
    UTextBlock* RoundStateText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("RoundStateText")));
    UOverlay* RangedCrosshairOverlay = Cast<UOverlay>(WidgetTree->FindWidget(TEXT("RangedCrosshairOverlay")));
    USizeBox* RangedCrosshairHorizontalBox = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("RangedCrosshairHorizontalBox")));
    USizeBox* RangedCrosshairVerticalBox = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("RangedCrosshairVerticalBox")));
    UBorder* RangedCrosshairHorizontalLine = Cast<UBorder>(WidgetTree->FindWidget(TEXT("RangedCrosshairHorizontalLine")));
    UBorder* RangedCrosshairVerticalLine = Cast<UBorder>(WidgetTree->FindWidget(TEXT("RangedCrosshairVerticalLine")));

    TestNotNull(TEXT("HealthBarSizeBox should exist"), HealthBarSizeBox);
    TestNotNull(TEXT("HealthBar should exist"), HealthBar);
    TestNotNull(TEXT("SprintBarSizeBox should exist"), SprintBarSizeBox);
    TestNotNull(TEXT("SprintBar should exist"), SprintBar);
    TestNotNull(TEXT("SprintText should exist"), SprintText);
    TestNotNull(TEXT("RangedCooldownBarSizeBox should exist"), RangedCooldownBarSizeBox);
    TestNotNull(TEXT("RangedCooldownBar should exist"), RangedCooldownBar);
    TestNotNull(TEXT("HealthText should exist"), HealthText);
    TestNotNull(TEXT("RangedCooldownText should exist"), RangedCooldownText);
    TestNotNull(TEXT("RoundScoreText should exist"), RoundScoreText);
    TestNotNull(TEXT("MatchScoreText should exist"), MatchScoreText);
    TestNotNull(TEXT("TimerText should exist"), TimerText);
    TestNotNull(TEXT("RoundStateText should exist"), RoundStateText);
    TestNotNull(TEXT("RangedCrosshairOverlay should exist"), RangedCrosshairOverlay);
    TestNotNull(TEXT("RangedCrosshairHorizontalBox should exist"), RangedCrosshairHorizontalBox);
    TestNotNull(TEXT("RangedCrosshairVerticalBox should exist"), RangedCrosshairVerticalBox);
    TestNotNull(TEXT("RangedCrosshairHorizontalLine should exist"), RangedCrosshairHorizontalLine);
    TestNotNull(TEXT("RangedCrosshairVerticalLine should exist"), RangedCrosshairVerticalLine);
    UTextBlock* ResultText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("ResultText")));
    UTextBlock* NextRoundText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("NextRoundText")));
    UTextBlock* LobbyControlsTitleText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("LobbyControlsTitleText")));
    UTextBlock* LobbyControlsKeyboardTitleText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("LobbyControlsKeyboardTitleText")));
    UTextBlock* LobbyControlsKeyboardMoveText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("LobbyControlsKeyboardMoveText")));
    UTextBlock* LobbyControlsKeyboardSprintText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("LobbyControlsKeyboardSprintText")));
    UTextBlock* LobbyControlsMouseTitleText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("LobbyControlsMouseTitleText")));
    UTextBlock* LobbyControlsMouseMeleeText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("LobbyControlsMouseMeleeText")));
    UTextBlock* LobbyControlsMouseRangedText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("LobbyControlsMouseRangedText")));
    UTextBlock* ConnectionStatusText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("ConnectionStatusText")));
    UEditableTextBox* LobbyNicknameTextBox = Cast<UEditableTextBox>(WidgetTree->FindWidget(TEXT("LobbyNicknameTextBox")));
    UEditableTextBox* JoinAddressTextBox = Cast<UEditableTextBox>(WidgetTree->FindWidget(TEXT("JoinAddressTextBox")));
    UButton* HostMatchButton = Cast<UButton>(WidgetTree->FindWidget(TEXT("HostMatchButton")));
    UButton* JoinByIpButton = Cast<UButton>(WidgetTree->FindWidget(TEXT("JoinByIpButton")));
    UTextBlock* LobbyReadyButtonText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("LobbyReadyButtonText")));
    UButton* SettingsResumeButton = Cast<UButton>(WidgetTree->FindWidget(TEXT("SettingsResumeButton")));
    UButton* SettingsQuitButton = Cast<UButton>(WidgetTree->FindWidget(TEXT("SettingsQuitButton")));

    TestTrue(TEXT("Health bar should use a thicker readability-first height"),
        HealthBarSizeBox && HealthBarSizeBox->GetHeightOverride() >= 20.0f);
    TestTrue(TEXT("Sprint bar should remain readable at HUD scale"),
        SprintBarSizeBox && SprintBarSizeBox->GetHeightOverride() >= 14.0f);
    TestTrue(TEXT("Ranged cooldown bar should remain readable at HUD scale"),
        RangedCooldownBarSizeBox && RangedCooldownBarSizeBox->GetHeightOverride() >= 14.0f);
    TestTrue(TEXT("Health text should be larger than default small body text"),
        HealthText && HealthText->GetFont().Size >= 18);
    TestTrue(TEXT("Ranged cooldown text should be larger than default small body text"),
        RangedCooldownText && RangedCooldownText->GetFont().Size >= 18);
    TestTrue(TEXT("Sprint text should be larger than default small body text"),
        SprintText && SprintText->GetFont().Size >= 18);
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
    TestNotNull(TEXT("LobbyControlsTitleText should exist"), LobbyControlsTitleText);
    TestNotNull(TEXT("LobbyControlsKeyboardTitleText should exist"), LobbyControlsKeyboardTitleText);
    TestNotNull(TEXT("LobbyControlsKeyboardMoveText should exist"), LobbyControlsKeyboardMoveText);
    TestNotNull(TEXT("LobbyControlsKeyboardSprintText should exist"), LobbyControlsKeyboardSprintText);
    TestNotNull(TEXT("LobbyControlsMouseTitleText should exist"), LobbyControlsMouseTitleText);
    TestNotNull(TEXT("LobbyControlsMouseMeleeText should exist"), LobbyControlsMouseMeleeText);
    TestNotNull(TEXT("LobbyControlsMouseRangedText should exist"), LobbyControlsMouseRangedText);
    TestNotNull(TEXT("ConnectionStatusText should exist"), ConnectionStatusText);
    TestNotNull(TEXT("LobbyNicknameTextBox should exist"), LobbyNicknameTextBox);
    TestNotNull(TEXT("JoinAddressTextBox should exist"), JoinAddressTextBox);
    TestNotNull(TEXT("HostMatchButton should exist"), HostMatchButton);
    TestNotNull(TEXT("JoinByIpButton should exist"), JoinByIpButton);
    TestNotNull(TEXT("LobbyReadyButtonText should exist"), LobbyReadyButtonText);
    TestNotNull(TEXT("SettingsResumeButton should exist"), SettingsResumeButton);
    TestNotNull(TEXT("SettingsQuitButton should exist"), SettingsQuitButton);
    TestEqual(TEXT("AnnouncementPanel should stay hidden until round end"),
        AnnouncementPanel ? AnnouncementPanel->GetVisibility() : ESlateVisibility::Visible,
        ESlateVisibility::Collapsed);
    TestEqual(TEXT("ResultText should stay hidden until round end"),
        ResultText ? ResultText->GetVisibility() : ESlateVisibility::Visible,
        ESlateVisibility::Collapsed);
    TestEqual(TEXT("NextRoundText should stay hidden until round end"),
        NextRoundText ? NextRoundText->GetVisibility() : ESlateVisibility::Visible,
        ESlateVisibility::Collapsed);
    TestEqual(TEXT("Crosshair should stay hidden until ranged charge is held"),
        RangedCrosshairOverlay ? RangedCrosshairOverlay->GetVisibility() : ESlateVisibility::Visible,
        ESlateVisibility::Collapsed);
    TestTrue(TEXT("ResultText should use a larger font than default status rows"),
        ResultText && ResultText->GetFont().Size >= 30);
    TestTrue(TEXT("NextRoundText should use a larger font than default status rows"),
        NextRoundText && NextRoundText->GetFont().Size >= 22);
    TestTrue(TEXT("Crosshair should remain prominent at screen center"),
        RangedCrosshairOverlay && RangedCrosshairOverlay->GetChildrenCount() == 2);
    TestTrue(TEXT("Crosshair horizontal line should be wider than it is tall"),
        RangedCrosshairHorizontalBox && RangedCrosshairHorizontalBox->GetWidthOverride() > RangedCrosshairHorizontalBox->GetHeightOverride());
    TestTrue(TEXT("Crosshair vertical line should be taller than it is wide"),
        RangedCrosshairVerticalBox && RangedCrosshairVerticalBox->GetHeightOverride() > RangedCrosshairVerticalBox->GetWidthOverride());
    UOverlaySlot* CrosshairRootSlot = RangedCrosshairOverlay ? Cast<UOverlaySlot>(RangedCrosshairOverlay->Slot) : nullptr;
    TestTrue(TEXT("Crosshair root should stay centered in the HUD overlay"),
        CrosshairRootSlot
        && CrosshairRootSlot->GetHorizontalAlignment() == HAlign_Center
        && CrosshairRootSlot->GetVerticalAlignment() == VAlign_Center);
    TestTrue(TEXT("ResultText should remain larger than the status text hierarchy"),
        ResultText && HealthText && ResultText->GetFont().Size > HealthText->GetFont().Size);
    TestTrue(TEXT("Status panel should use a visible dark backing"),
        StatusPanel && StatusPanel->GetBrushColor().A >= 0.7f);
    UOverlaySlot* StatusPanelSlot = StatusPanel ? Cast<UOverlaySlot>(StatusPanel->Slot) : nullptr;
    TestTrue(TEXT("Status panel should stay pinned to the bottom center"),
        StatusPanelSlot
        && StatusPanelSlot->GetHorizontalAlignment() == HAlign_Center
        && StatusPanelSlot->GetVerticalAlignment() == VAlign_Bottom);
    UOverlaySlot* CountdownPanelSlot = CountdownPanel ? Cast<UOverlaySlot>(CountdownPanel->Slot) : nullptr;
    TestTrue(TEXT("Countdown panel should stay pinned to the top center"),
        CountdownPanelSlot
        && CountdownPanelSlot->GetHorizontalAlignment() == HAlign_Center
        && CountdownPanelSlot->GetVerticalAlignment() == VAlign_Top);
    UOverlaySlot* InfoPanelSlot = InfoPanel ? Cast<UOverlaySlot>(InfoPanel->Slot) : nullptr;
    TestTrue(TEXT("Info panel should stay pinned to the upper-left corner"),
        InfoPanelSlot
        && InfoPanelSlot->GetHorizontalAlignment() == HAlign_Left
        && InfoPanelSlot->GetVerticalAlignment() == VAlign_Top);
    TestTrue(TEXT("Announcement panel should use a visible dark backing"),
        AnnouncementPanel && AnnouncementPanel->GetBrushColor().A >= 0.8f);
    UOverlaySlot* LobbyControlsPanelSlot = LobbyControlsPanel ? Cast<UOverlaySlot>(LobbyControlsPanel->Slot) : nullptr;
    TestTrue(TEXT("Lobby controls panel should stay pinned near the bottom center"),
        LobbyControlsPanelSlot
        && LobbyControlsPanelSlot->GetHorizontalAlignment() == HAlign_Center
        && LobbyControlsPanelSlot->GetVerticalAlignment() == VAlign_Bottom);
    TestEqual(TEXT("Lobby controls panel should stay hidden until the lobby is shown"),
        LobbyControlsPanel ? LobbyControlsPanel->GetVisibility() : ESlateVisibility::Visible,
        ESlateVisibility::Collapsed);
    TestEqual(TEXT("Settings panel should stay hidden until the menu is opened"),
        SettingsPanel ? SettingsPanel->GetVisibility() : ESlateVisibility::Visible,
        ESlateVisibility::Collapsed);
    TestTrue(TEXT("Lobby keyboard card should use a visible dark backing"),
        LobbyKeyboardCard && LobbyKeyboardCard->GetBrushColor().A >= 0.85f);
    TestTrue(TEXT("Lobby mouse card should use a visible dark backing"),
        LobbyMouseCard && LobbyMouseCard->GetBrushColor().A >= 0.85f);
    TestTrue(TEXT("Keyboard controls should describe WASD movement"),
        LobbyControlsKeyboardMoveText && LobbyControlsKeyboardMoveText->GetText().ToString().Contains(TEXT("Move")));
    TestTrue(TEXT("Keyboard controls should describe Shift dash"),
        LobbyControlsKeyboardSprintText && LobbyControlsKeyboardSprintText->GetText().ToString().Contains(TEXT("Dash")));
    TestTrue(TEXT("Mouse controls should describe melee attack"),
        LobbyControlsMouseMeleeText && LobbyControlsMouseMeleeText->GetText().ToString().Contains(TEXT("Melee Attack")));
    TestTrue(TEXT("Mouse controls should describe ranged charge attack"),
        LobbyControlsMouseRangedText && LobbyControlsMouseRangedText->GetText().ToString().Contains(TEXT("Ranged Attack (Charge)")));
    TestTrue(TEXT("Connection status should sit inside the main lobby panel"),
        ConnectionStatusText && ConnectionStatusText->Slot && ConnectionStatusText->Slot->Parent == LobbyBox);
    TestTrue(TEXT("Join address text box should sit inside the main lobby panel"),
        JoinAddressTextBox && JoinAddressTextBox->Slot && JoinAddressTextBox->Slot->Parent == LobbyBox);
    TestTrue(TEXT("Timer text should sit inside the dedicated countdown panel"),
        TimerText && TimerText->Slot && TimerText->Slot->Parent != InfoBox);
    TestEqual(TEXT("Lobby start button should use the new explicit start label"),
        LobbyReadyButtonText ? LobbyReadyButtonText->GetText().ToString() : FString(),
        FString(TEXT("Start Match")));
    TestTrue(TEXT("Join address text box should guide direct IP input"),
        JoinAddressTextBox && JoinAddressTextBox->GetHintText().ToString().Contains(TEXT("7777")));
    return true;
}
