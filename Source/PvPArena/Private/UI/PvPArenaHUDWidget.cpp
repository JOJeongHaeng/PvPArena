#include "UI/PvPArenaHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "Game/PvPArenaGameState.h"
#include "Game/PvPArenaPlayerController.h"
#include "Game/PvPArenaPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Combat/PvPCombatComponent.h"
#include "Player/PvPArenaCharacter.h"
#include "TimerManager.h"
#include "Widgets/SWidget.h"

TSharedRef<SWidget> UPvPArenaHUDWidget::RebuildWidget()
{
    BuildWidgetTree();
    return Super::RebuildWidget();
}

void UPvPArenaHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BuildWidgetTree();
    RefreshWidgetData();
    RefreshCrosshairVisibility();
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(RefreshTimerHandle, this, &UPvPArenaHUDWidget::RefreshWidgetData, 0.15f, true);
    }
}

void UPvPArenaHUDWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RefreshTimerHandle);
    }

    Super::NativeDestruct();
}

void UPvPArenaHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    RefreshCrosshairVisibility();
}

void UPvPArenaHUDWidget::BuildWidgetTree()
{
    if (!WidgetTree)
    {
        return;
    }

    if (RootOverlay)
    {
        return;
    }

    RootOverlay = Cast<UOverlay>(WidgetTree->RootWidget);
    if (!RootOverlay)
    {
        RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
        WidgetTree->RootWidget = RootOverlay;
    }

    StatusPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StatusPanel"));
    InfoPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InfoPanel"));
    AnnouncementPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("AnnouncementPanel"));
    LobbyPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LobbyPanel"));
    LobbyControlsPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LobbyControlsPanel"));
    MatchResultPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MatchResultPanel"));
    StatusBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatusBox"));
    InfoBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InfoBox"));
    AnnouncementBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("AnnouncementBox"));
    LobbyBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyBox"));
    LobbyControlsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyControlsBox"));
    LobbyControlsCardsBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("LobbyControlsCardsBox"));
    MatchResultBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MatchResultBox"));
    LobbyKeyboardCard = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LobbyKeyboardCard"));
    LobbyMouseCard = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LobbyMouseCard"));
    LobbyKeyboardCardBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyKeyboardCardBox"));
    LobbyMouseCardBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyMouseCardBox"));
    HealthBarSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("HealthBarSizeBox"));
    HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
    SprintBarSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SprintBarSizeBox"));
    SprintBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("SprintBar"));
    RangedCooldownBarSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RangedCooldownBarSizeBox"));
    RangedCooldownBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("RangedCooldownBar"));
    HealthText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthText"));
    SprintText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SprintText"));
    RangedCooldownText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RangedCooldownText"));
    RoundScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundScoreText"));
    MatchScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MatchScoreText"));
    TimerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TimerText"));
    RoundStateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundStateText"));
    ResultText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ResultText"));
    NextRoundText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NextRoundText"));
    LobbyTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyTitleText"));
    LobbyStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyStatusText"));
    LobbyReadyButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LobbyReadyButton"));
    LobbyReadyButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyReadyButtonText"));
    LobbyControlsTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsTitleText"));
    LobbyControlsKeyboardTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsKeyboardTitleText"));
    LobbyControlsKeyboardMoveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsKeyboardMoveText"));
    LobbyControlsKeyboardSprintText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsKeyboardSprintText"));
    LobbyControlsMouseTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsMouseTitleText"));
    LobbyControlsMouseMeleeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsMouseMeleeText"));
    LobbyControlsMouseRangedText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsMouseRangedText"));
    MatchResultTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MatchResultTitleText"));
    MatchResultSummaryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MatchResultSummaryText"));
    RangedCrosshairOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RangedCrosshairOverlay"));
    RangedCrosshairHorizontalBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RangedCrosshairHorizontalBox"));
    RangedCrosshairVerticalBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RangedCrosshairVerticalBox"));
    RangedCrosshairHorizontalLine = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RangedCrosshairHorizontalLine"));
    RangedCrosshairVerticalLine = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RangedCrosshairVerticalLine"));

    if (!RootOverlay || !StatusPanel || !InfoPanel || !AnnouncementPanel || !LobbyPanel || !LobbyControlsPanel || !MatchResultPanel || !StatusBox || !InfoBox || !AnnouncementBox || !LobbyBox || !LobbyControlsBox || !LobbyControlsCardsBox || !MatchResultBox || !LobbyKeyboardCard || !LobbyMouseCard || !LobbyKeyboardCardBox || !LobbyMouseCardBox || !HealthBarSizeBox || !HealthBar || !SprintBarSizeBox || !SprintBar || !RangedCooldownBarSizeBox || !RangedCooldownBar || !HealthText || !SprintText || !RangedCooldownText || !RoundScoreText || !MatchScoreText || !TimerText || !RoundStateText || !ResultText || !NextRoundText || !LobbyTitleText || !LobbyStatusText || !LobbyReadyButton || !LobbyReadyButtonText || !LobbyControlsTitleText || !LobbyControlsKeyboardTitleText || !LobbyControlsKeyboardMoveText || !LobbyControlsKeyboardSprintText || !LobbyControlsMouseTitleText || !LobbyControlsMouseMeleeText || !LobbyControlsMouseRangedText || !MatchResultTitleText || !MatchResultSummaryText || !RangedCrosshairOverlay || !RangedCrosshairHorizontalBox || !RangedCrosshairVerticalBox || !RangedCrosshairHorizontalLine || !RangedCrosshairVerticalLine)
    {
        return;
    }

    StatusPanel->SetContent(StatusBox);
    StatusPanel->SetPadding(FMargin(18.0f, 16.0f, 18.0f, 16.0f));
    StatusPanel->SetBrushColor(FLinearColor(0.03f, 0.05f, 0.08f, 0.78f));

    InfoPanel->SetContent(InfoBox);
    InfoPanel->SetPadding(FMargin(18.0f, 16.0f, 18.0f, 16.0f));
    InfoPanel->SetBrushColor(FLinearColor(0.03f, 0.05f, 0.08f, 0.78f));

    AnnouncementPanel->SetContent(AnnouncementBox);
    AnnouncementPanel->SetPadding(FMargin(28.0f, 20.0f, 28.0f, 20.0f));
    AnnouncementPanel->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.06f, 0.82f));
    AnnouncementPanel->SetVisibility(ESlateVisibility::Collapsed);

    LobbyPanel->SetContent(LobbyBox);
    LobbyPanel->SetPadding(FMargin(34.0f, 26.0f, 34.0f, 26.0f));
    LobbyPanel->SetBrushColor(FLinearColor(0.02f, 0.05f, 0.08f, 0.84f));
    LobbyPanel->SetVisibility(ESlateVisibility::Collapsed);

    LobbyControlsPanel->SetContent(LobbyControlsBox);
    LobbyControlsPanel->SetPadding(FMargin(28.0f, 20.0f, 28.0f, 20.0f));
    LobbyControlsPanel->SetBrushColor(FLinearColor(0.02f, 0.05f, 0.08f, 0.76f));
    LobbyControlsPanel->SetVisibility(ESlateVisibility::Collapsed);

    MatchResultPanel->SetContent(MatchResultBox);
    MatchResultPanel->SetPadding(FMargin(34.0f, 26.0f, 34.0f, 26.0f));
    MatchResultPanel->SetBrushColor(FLinearColor(0.08f, 0.04f, 0.02f, 0.86f));
    MatchResultPanel->SetVisibility(ESlateVisibility::Collapsed);

    UOverlaySlot* StatusBoxSlot = RootOverlay->AddChildToOverlay(StatusPanel);
    UOverlaySlot* InfoBoxSlot = RootOverlay->AddChildToOverlay(InfoPanel);
    UOverlaySlot* AnnouncementBoxSlot = RootOverlay->AddChildToOverlay(AnnouncementPanel);
    UOverlaySlot* LobbyBoxSlot = RootOverlay->AddChildToOverlay(LobbyPanel);
    UOverlaySlot* LobbyControlsBoxSlot = RootOverlay->AddChildToOverlay(LobbyControlsPanel);
    UOverlaySlot* MatchResultBoxSlot = RootOverlay->AddChildToOverlay(MatchResultPanel);
    UOverlaySlot* RangedCrosshairSlot = RootOverlay->AddChildToOverlay(RangedCrosshairOverlay);

    if (StatusBoxSlot)
    {
        StatusBoxSlot->SetHorizontalAlignment(HAlign_Right);
        StatusBoxSlot->SetVerticalAlignment(VAlign_Top);
        StatusBoxSlot->SetPadding(FMargin(0.0f, 40.0f, 40.0f, 0.0f));
    }

    if (InfoBoxSlot)
    {
        InfoBoxSlot->SetHorizontalAlignment(HAlign_Left);
        InfoBoxSlot->SetVerticalAlignment(VAlign_Top);
        InfoBoxSlot->SetPadding(FMargin(40.0f, 40.0f, 0.0f, 0.0f));
    }

    if (AnnouncementBoxSlot)
    {
        AnnouncementBoxSlot->SetHorizontalAlignment(HAlign_Center);
        AnnouncementBoxSlot->SetVerticalAlignment(VAlign_Center);
        AnnouncementBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 80.0f));
    }

    if (LobbyBoxSlot)
    {
        LobbyBoxSlot->SetHorizontalAlignment(HAlign_Center);
        LobbyBoxSlot->SetVerticalAlignment(VAlign_Center);
    }

    if (LobbyControlsBoxSlot)
    {
        LobbyControlsBoxSlot->SetHorizontalAlignment(HAlign_Center);
        LobbyControlsBoxSlot->SetVerticalAlignment(VAlign_Bottom);
        LobbyControlsBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 36.0f));
    }

    if (MatchResultBoxSlot)
    {
        MatchResultBoxSlot->SetHorizontalAlignment(HAlign_Center);
        MatchResultBoxSlot->SetVerticalAlignment(VAlign_Center);
    }

    if (RangedCrosshairSlot)
    {
        RangedCrosshairSlot->SetHorizontalAlignment(HAlign_Center);
        RangedCrosshairSlot->SetVerticalAlignment(VAlign_Center);
    }

    RangedCrosshairHorizontalBox->SetWidthOverride(18.0f);
    RangedCrosshairHorizontalBox->SetHeightOverride(2.0f);
    RangedCrosshairHorizontalBox->SetContent(RangedCrosshairHorizontalLine);
    RangedCrosshairVerticalBox->SetWidthOverride(2.0f);
    RangedCrosshairVerticalBox->SetHeightOverride(18.0f);
    RangedCrosshairVerticalBox->SetContent(RangedCrosshairVerticalLine);

    UOverlaySlot* HorizontalCrosshairSlot = RangedCrosshairOverlay->AddChildToOverlay(RangedCrosshairHorizontalBox);
    UOverlaySlot* VerticalCrosshairSlot = RangedCrosshairOverlay->AddChildToOverlay(RangedCrosshairVerticalBox);
    if (HorizontalCrosshairSlot)
    {
        HorizontalCrosshairSlot->SetHorizontalAlignment(HAlign_Center);
        HorizontalCrosshairSlot->SetVerticalAlignment(VAlign_Center);
        HorizontalCrosshairSlot->SetPadding(FMargin(-9.0f, 0.0f, -9.0f, 0.0f));
    }

    if (VerticalCrosshairSlot)
    {
        VerticalCrosshairSlot->SetHorizontalAlignment(HAlign_Center);
        VerticalCrosshairSlot->SetVerticalAlignment(VAlign_Center);
        VerticalCrosshairSlot->SetPadding(FMargin(0.0f, -9.0f, 0.0f, -9.0f));
    }

    HealthBarSizeBox->SetWidthOverride(280.0f);
    HealthBarSizeBox->SetHeightOverride(22.0f);
    HealthBarSizeBox->SetContent(HealthBar);

    SprintBarSizeBox->SetWidthOverride(280.0f);
    SprintBarSizeBox->SetHeightOverride(16.0f);
    SprintBarSizeBox->SetContent(SprintBar);

    RangedCooldownBarSizeBox->SetWidthOverride(280.0f);
    RangedCooldownBarSizeBox->SetHeightOverride(16.0f);
    RangedCooldownBarSizeBox->SetContent(RangedCooldownBar);

    HealthBar->SetPercent(1.0f);
    HealthBar->SetFillColorAndOpacity(FLinearColor(0.9f, 0.2f, 0.2f, 1.0f));
    SprintBar->SetPercent(1.0f);
    SprintBar->SetFillColorAndOpacity(FLinearColor(0.95f, 0.82f, 0.15f, 1.0f));
    RangedCooldownBar->SetPercent(1.0f);
    RangedCooldownBar->SetFillColorAndOpacity(FLinearColor(0.2f, 0.75f, 1.0f, 1.0f));
    UVerticalBoxSlot* HealthBarSlot = StatusBox->AddChildToVerticalBox(HealthBarSizeBox);
    UVerticalBoxSlot* HealthTextSlot = StatusBox->AddChildToVerticalBox(HealthText);
    UVerticalBoxSlot* SprintBarSlot = StatusBox->AddChildToVerticalBox(SprintBarSizeBox);
    UVerticalBoxSlot* SprintTextSlot = StatusBox->AddChildToVerticalBox(SprintText);
    UVerticalBoxSlot* RangedCooldownBarSlot = StatusBox->AddChildToVerticalBox(RangedCooldownBarSizeBox);
    UVerticalBoxSlot* RangedCooldownTextSlot = StatusBox->AddChildToVerticalBox(RangedCooldownText);
    UVerticalBoxSlot* RoundScoreTextSlot = InfoBox->AddChildToVerticalBox(RoundScoreText);
    UVerticalBoxSlot* MatchScoreTextSlot = InfoBox->AddChildToVerticalBox(MatchScoreText);
    UVerticalBoxSlot* TimerTextSlot = InfoBox->AddChildToVerticalBox(TimerText);
    UVerticalBoxSlot* RoundStateTextSlot = InfoBox->AddChildToVerticalBox(RoundStateText);
    UVerticalBoxSlot* ResultTextSlot = AnnouncementBox->AddChildToVerticalBox(ResultText);
    UVerticalBoxSlot* NextRoundTextSlot = AnnouncementBox->AddChildToVerticalBox(NextRoundText);
    UVerticalBoxSlot* LobbyTitleTextSlot = LobbyBox->AddChildToVerticalBox(LobbyTitleText);
    UVerticalBoxSlot* LobbyStatusTextSlot = LobbyBox->AddChildToVerticalBox(LobbyStatusText);
    UVerticalBoxSlot* LobbyControlsTitleTextSlot = LobbyControlsBox->AddChildToVerticalBox(LobbyControlsTitleText);
    UVerticalBoxSlot* LobbyControlsCardsBoxSlot = LobbyControlsBox->AddChildToVerticalBox(LobbyControlsCardsBox);
    UHorizontalBoxSlot* LobbyKeyboardCardSlot = LobbyControlsCardsBox->AddChildToHorizontalBox(LobbyKeyboardCard);
    UHorizontalBoxSlot* LobbyMouseCardSlot = LobbyControlsCardsBox->AddChildToHorizontalBox(LobbyMouseCard);
    UVerticalBoxSlot* LobbyControlsKeyboardTitleTextSlot = LobbyKeyboardCardBox->AddChildToVerticalBox(LobbyControlsKeyboardTitleText);
    UVerticalBoxSlot* LobbyControlsKeyboardMoveTextSlot = LobbyKeyboardCardBox->AddChildToVerticalBox(LobbyControlsKeyboardMoveText);
    UVerticalBoxSlot* LobbyControlsKeyboardSprintTextSlot = LobbyKeyboardCardBox->AddChildToVerticalBox(LobbyControlsKeyboardSprintText);
    UVerticalBoxSlot* LobbyControlsMouseTitleTextSlot = LobbyMouseCardBox->AddChildToVerticalBox(LobbyControlsMouseTitleText);
    UVerticalBoxSlot* LobbyControlsMouseMeleeTextSlot = LobbyMouseCardBox->AddChildToVerticalBox(LobbyControlsMouseMeleeText);
    UVerticalBoxSlot* LobbyControlsMouseRangedTextSlot = LobbyMouseCardBox->AddChildToVerticalBox(LobbyControlsMouseRangedText);
    UVerticalBoxSlot* MatchResultTitleTextSlot = MatchResultBox->AddChildToVerticalBox(MatchResultTitleText);
    UVerticalBoxSlot* MatchResultSummaryTextSlot = MatchResultBox->AddChildToVerticalBox(MatchResultSummaryText);

    if (HealthBarSlot)
    {
        HealthBarSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    }

    if (HealthTextSlot)
    {
        HealthTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    HealthText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    HealthText->SetFont(FSlateFontInfo(HealthText->GetFont().FontObject, 20, HealthText->GetFont().TypefaceFontName));
    HealthText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (SprintBarSlot)
    {
        SprintBarSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }

    if (SprintTextSlot)
    {
        SprintTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    SprintText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.88f, 0.35f, 1.0f)));
    SprintText->SetFont(FSlateFontInfo(SprintText->GetFont().FontObject, 18, SprintText->GetFont().TypefaceFontName));
    SprintText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (RangedCooldownBarSlot)
    {
        RangedCooldownBarSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }

    if (RangedCooldownTextSlot)
    {
        RangedCooldownTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    RangedCooldownText->SetColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.9f, 1.0f, 1.0f)));
    RangedCooldownText->SetFont(FSlateFontInfo(RangedCooldownText->GetFont().FontObject, 18, RangedCooldownText->GetFont().TypefaceFontName));
    RangedCooldownText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (RoundScoreTextSlot)
    {
        RoundScoreTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
    }
    RoundScoreText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
    RoundScoreText->SetFont(FSlateFontInfo(RoundScoreText->GetFont().FontObject, 18, RoundScoreText->GetFont().TypefaceFontName));
    RoundScoreText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (MatchScoreTextSlot)
    {
        MatchScoreTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
    }
    MatchScoreText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.2f, 1.0f)));
    MatchScoreText->SetFont(FSlateFontInfo(MatchScoreText->GetFont().FontObject, 18, MatchScoreText->GetFont().TypefaceFontName));
    MatchScoreText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (TimerTextSlot)
    {
        TimerTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
    }
    TimerText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 1.0f, 0.55f, 1.0f)));
    TimerText->SetFont(FSlateFontInfo(TimerText->GetFont().FontObject, 18, TimerText->GetFont().TypefaceFontName));
    TimerText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (RoundStateTextSlot)
    {
        RoundStateTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    RoundStateText->SetColorAndOpacity(FSlateColor(FLinearColor(0.45f, 0.9f, 1.0f, 1.0f)));
    RoundStateText->SetFont(FSlateFontInfo(RoundStateText->GetFont().FontObject, 18, RoundStateText->GetFont().TypefaceFontName));
    RoundStateText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (ResultTextSlot)
    {
        ResultTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    }
    ResultText->SetJustification(ETextJustify::Center);
    ResultText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
    ResultText->SetFont(FSlateFontInfo(ResultText->GetFont().FontObject, 42, ResultText->GetFont().TypefaceFontName));
    ResultText->SetShadowOffset(FVector2D(2.0f, 2.0f));
    ResultText->SetVisibility(ESlateVisibility::Collapsed);

    if (NextRoundTextSlot)
    {
        NextRoundTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    NextRoundText->SetJustification(ETextJustify::Center);
    NextRoundText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.6f, 0.0f, 1.0f)));
    NextRoundText->SetFont(FSlateFontInfo(NextRoundText->GetFont().FontObject, 28, NextRoundText->GetFont().TypefaceFontName));
    NextRoundText->SetShadowOffset(FVector2D(1.0f, 1.0f));
    NextRoundText->SetVisibility(ESlateVisibility::Collapsed);

    if (LobbyTitleTextSlot)
    {
        LobbyTitleTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    }
    LobbyTitleText->SetJustification(ETextJustify::Center);
    LobbyTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.96f, 1.0f, 1.0f)));
    LobbyTitleText->SetFont(FSlateFontInfo(LobbyTitleText->GetFont().FontObject, 38, LobbyTitleText->GetFont().TypefaceFontName));
    LobbyTitleText->SetShadowOffset(FVector2D(2.0f, 2.0f));
    LobbyTitleText->SetText(FText::FromString(TEXT("Lobby")));

    if (LobbyStatusTextSlot)
    {
        LobbyStatusTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    LobbyStatusText->SetJustification(ETextJustify::Center);
    LobbyStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.65f, 0.92f, 1.0f, 1.0f)));
    LobbyStatusText->SetFont(FSlateFontInfo(LobbyStatusText->GetFont().FontObject, 24, LobbyStatusText->GetFont().TypefaceFontName));
    LobbyStatusText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    LobbyReadyButton->SetContent(LobbyReadyButtonText);
    LobbyReadyButton->SetBackgroundColor(FLinearColor(0.15f, 0.55f, 0.82f, 1.0f));
    LobbyReadyButton->OnClicked.AddDynamic(this, &UPvPArenaHUDWidget::HandleLobbyReadyButtonClicked);
    UVerticalBoxSlot* LobbyReadyButtonSlot = LobbyBox->AddChildToVerticalBox(LobbyReadyButton);
    if (LobbyReadyButtonSlot)
    {
        LobbyReadyButtonSlot->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 0.0f));
        LobbyReadyButtonSlot->SetHorizontalAlignment(HAlign_Center);
    }

    LobbyReadyButtonText->SetJustification(ETextJustify::Center);
    LobbyReadyButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    LobbyReadyButtonText->SetFont(FSlateFontInfo(LobbyReadyButtonText->GetFont().FontObject, 22, LobbyReadyButtonText->GetFont().TypefaceFontName));
    LobbyReadyButtonText->SetText(FText::FromString(TEXT("Ready")));

    if (LobbyControlsTitleTextSlot)
    {
        LobbyControlsTitleTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
        LobbyControlsTitleTextSlot->SetHorizontalAlignment(HAlign_Center);
    }

    if (LobbyControlsCardsBoxSlot)
    {
        LobbyControlsCardsBoxSlot->SetHorizontalAlignment(HAlign_Center);
    }
    LobbyControlsTitleText->SetJustification(ETextJustify::Center);
    LobbyControlsTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.98f, 1.0f, 1.0f)));
    LobbyControlsTitleText->SetFont(FSlateFontInfo(LobbyControlsTitleText->GetFont().FontObject, 24, LobbyControlsTitleText->GetFont().TypefaceFontName));
    LobbyControlsTitleText->SetText(FText::FromString(TEXT("Controls")));

    LobbyKeyboardCard->SetContent(LobbyKeyboardCardBox);
    LobbyKeyboardCard->SetPadding(FMargin(18.0f, 14.0f, 18.0f, 14.0f));
    LobbyKeyboardCard->SetBrushColor(FLinearColor(0.07f, 0.12f, 0.18f, 0.92f));
    LobbyMouseCard->SetContent(LobbyMouseCardBox);
    LobbyMouseCard->SetPadding(FMargin(18.0f, 14.0f, 18.0f, 14.0f));
    LobbyMouseCard->SetBrushColor(FLinearColor(0.11f, 0.09f, 0.07f, 0.92f));

    if (LobbyKeyboardCardSlot)
    {
        LobbyKeyboardCardSlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));
    }

    if (LobbyMouseCardSlot)
    {
        LobbyMouseCardSlot->SetPadding(FMargin(10.0f, 0.0f, 0.0f, 0.0f));
    }

    if (LobbyControlsKeyboardTitleTextSlot)
    {
        LobbyControlsKeyboardTitleTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    LobbyControlsKeyboardTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.98f, 1.0f, 1.0f)));
    LobbyControlsKeyboardTitleText->SetFont(FSlateFontInfo(LobbyControlsKeyboardTitleText->GetFont().FontObject, 18, LobbyControlsKeyboardTitleText->GetFont().TypefaceFontName));
    LobbyControlsKeyboardTitleText->SetText(FText::FromString(TEXT("Keyboard")));

    if (LobbyControlsKeyboardMoveTextSlot)
    {
        LobbyControlsKeyboardMoveTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
    }
    LobbyControlsKeyboardMoveText->SetColorAndOpacity(FSlateColor(FLinearColor(0.82f, 0.92f, 1.0f, 1.0f)));
    LobbyControlsKeyboardMoveText->SetFont(FSlateFontInfo(LobbyControlsKeyboardMoveText->GetFont().FontObject, 17, LobbyControlsKeyboardMoveText->GetFont().TypefaceFontName));
    LobbyControlsKeyboardMoveText->SetText(FText::FromString(TEXT("[ W ]\n[ A ] [ S ] [ D ]   Move")));

    if (LobbyControlsKeyboardSprintTextSlot)
    {
        LobbyControlsKeyboardSprintTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    LobbyControlsKeyboardSprintText->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.86f, 0.35f, 1.0f)));
    LobbyControlsKeyboardSprintText->SetFont(FSlateFontInfo(LobbyControlsKeyboardSprintText->GetFont().FontObject, 17, LobbyControlsKeyboardSprintText->GetFont().TypefaceFontName));
    LobbyControlsKeyboardSprintText->SetText(FText::FromString(TEXT("[ Shift ]   Sprint")));

    if (LobbyControlsMouseTitleTextSlot)
    {
        LobbyControlsMouseTitleTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    LobbyControlsMouseTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.97f, 0.92f, 1.0f)));
    LobbyControlsMouseTitleText->SetFont(FSlateFontInfo(LobbyControlsMouseTitleText->GetFont().FontObject, 18, LobbyControlsMouseTitleText->GetFont().TypefaceFontName));
    LobbyControlsMouseTitleText->SetText(FText::FromString(TEXT("Mouse")));

    if (LobbyControlsMouseMeleeTextSlot)
    {
        LobbyControlsMouseMeleeTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
    }
    LobbyControlsMouseMeleeText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.9f, 0.82f, 1.0f)));
    LobbyControlsMouseMeleeText->SetFont(FSlateFontInfo(LobbyControlsMouseMeleeText->GetFont().FontObject, 17, LobbyControlsMouseMeleeText->GetFont().TypefaceFontName));
    LobbyControlsMouseMeleeText->SetText(FText::FromString(TEXT("[ LMB ]   Melee Attack")));

    if (LobbyControlsMouseRangedTextSlot)
    {
        LobbyControlsMouseRangedTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    LobbyControlsMouseRangedText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.82f, 0.62f, 1.0f)));
    LobbyControlsMouseRangedText->SetFont(FSlateFontInfo(LobbyControlsMouseRangedText->GetFont().FontObject, 17, LobbyControlsMouseRangedText->GetFont().TypefaceFontName));
    LobbyControlsMouseRangedText->SetText(FText::FromString(TEXT("[ RMB ]   Ranged Attack (Charge)")));

    if (MatchResultTitleTextSlot)
    {
        MatchResultTitleTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    }
    MatchResultTitleText->SetJustification(ETextJustify::Center);
    MatchResultTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.86f, 0.25f, 1.0f)));
    MatchResultTitleText->SetFont(FSlateFontInfo(MatchResultTitleText->GetFont().FontObject, 40, MatchResultTitleText->GetFont().TypefaceFontName));
    MatchResultTitleText->SetShadowOffset(FVector2D(2.0f, 2.0f));
    MatchResultTitleText->SetVisibility(ESlateVisibility::Collapsed);

    if (MatchResultSummaryTextSlot)
    {
        MatchResultSummaryTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    MatchResultSummaryText->SetJustification(ETextJustify::Center);
    MatchResultSummaryText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.92f, 0.85f, 1.0f)));
    MatchResultSummaryText->SetFont(FSlateFontInfo(MatchResultSummaryText->GetFont().FontObject, 24, MatchResultSummaryText->GetFont().TypefaceFontName));
    MatchResultSummaryText->SetShadowOffset(FVector2D(1.0f, 1.0f));
    MatchResultSummaryText->SetVisibility(ESlateVisibility::Collapsed);

    RangedCrosshairHorizontalLine->SetBrushColor(FLinearColor(0.95f, 0.98f, 1.0f, 0.95f));
    RangedCrosshairVerticalLine->SetBrushColor(FLinearColor(0.95f, 0.98f, 1.0f, 0.95f));
    RangedCrosshairOverlay->SetVisibility(ESlateVisibility::Collapsed);
}

void UPvPArenaHUDWidget::BuildHealthDisplayState(const APvPArenaCharacter* Character, float& OutHealthPercent, FString& OutHealthLabel)
{
    if (!Character)
    {
        OutHealthPercent = 0.0f;
        OutHealthLabel = TEXT("HP: --");
        return;
    }

    const float MaxHp = FMath::Max(1.0f, Character->GetMaxHealth());
    const float CurrentHp = Character->GetCurrentHealth();
    OutHealthPercent = CurrentHp / MaxHp;
    OutHealthLabel = FString::Printf(TEXT("HP: %.0f / %.0f"), CurrentHp, MaxHp);
}

void UPvPArenaHUDWidget::BuildSprintDisplayState(const APvPArenaCharacter* Character, float& OutSprintPercent, FString& OutSprintLabel)
{
    if (!Character)
    {
        OutSprintPercent = 0.0f;
        OutSprintLabel = TEXT("Sprint: --");
        return;
    }

    OutSprintPercent = Character->GetSprintEnergyAlpha();
    OutSprintLabel = FString::Printf(
        TEXT("Sprint: %.1fs %s"),
        Character->GetCurrentSprintEnergySeconds(),
        Character->IsSprinting() ? TEXT("(Boost)") : TEXT("(Charging)"));
}

void UPvPArenaHUDWidget::BuildRangedCooldownDisplayState(const UPvPCombatComponent* CombatComponent, float NowSeconds, float& OutCooldownPercent, FString& OutCooldownLabel)
{
    if (!CombatComponent)
    {
        OutCooldownPercent = 0.0f;
        OutCooldownLabel = TEXT("Ranged: --");
        return;
    }

    const float RemainingCooldown = CombatComponent->GetRemainingRangedCooldown(NowSeconds);
    OutCooldownPercent = CombatComponent->GetRangedCooldownAlpha(NowSeconds);
    OutCooldownLabel = RemainingCooldown > 0.0f
        ? FString::Printf(TEXT("Ranged: %.1fs"), RemainingCooldown)
        : FString(TEXT("Ranged: Ready"));
}

ESlateVisibility UPvPArenaHUDWidget::BuildRangedCrosshairVisibilityState(const APvPArenaCharacter* Character)
{
    return Character && Character->IsRangedChargeInputHeld()
        ? ESlateVisibility::Visible
        : ESlateVisibility::Collapsed;
}

void UPvPArenaHUDWidget::RefreshWidgetData()
{
    APlayerController* PlayerController = GetOwningPlayer();
    if (!PlayerController)
    {
        return;
    }

    float HealthPercent = 0.0f;
    FString HealthLabel;
    const APvPArenaCharacter* Character = Cast<APvPArenaCharacter>(PlayerController->GetPawn());
    BuildHealthDisplayState(Character, HealthPercent, HealthLabel);

    float SprintPercent = 0.0f;
    FString SprintLabel;
    BuildSprintDisplayState(Character, SprintPercent, SprintLabel);

    float RangedCooldownPercent = 0.0f;
    FString RangedCooldownLabel;
    const APvPArenaGameState* PvPGameState = GetWorld() ? GetWorld()->GetGameState<APvPArenaGameState>() : nullptr;
    const float CooldownNowSeconds = PvPGameState ? PvPGameState->GetServerWorldTimeSeconds() : (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f);
    BuildRangedCooldownDisplayState(
        Character ? Character->GetCombatComponent() : nullptr,
        CooldownNowSeconds,
        RangedCooldownPercent,
        RangedCooldownLabel);

    RefreshCrosshairVisibility();

    if (HealthBar)
    {
        HealthBar->SetPercent(HealthPercent);
    }

    if (HealthText)
    {
        HealthText->SetText(FText::FromString(HealthLabel));
    }

    if (SprintBar)
    {
        SprintBar->SetPercent(SprintPercent);
    }

    if (SprintText)
    {
        SprintText->SetText(FText::FromString(SprintLabel));
    }

    if (RangedCooldownBar)
    {
        RangedCooldownBar->SetPercent(RangedCooldownPercent);
    }

    if (RangedCooldownText)
    {
        RangedCooldownText->SetText(FText::FromString(RangedCooldownLabel));
    }

    if (const APvPArenaPlayerState* PvPPlayerState = PlayerController->GetPlayerState<APvPArenaPlayerState>())
    {
        if (RoundScoreText)
        {
            RoundScoreText->SetText(FText::FromString(
                FString::Printf(TEXT("Round K / D: %d / %d"), PvPPlayerState->GetRoundKills(), PvPPlayerState->GetRoundDeaths())));
        }

        if (MatchScoreText)
        {
            MatchScoreText->SetText(FText::FromString(
                FString::Printf(
                    TEXT("Rounds: %d / %d | Match K / D: %d / %d"),
                    PvPPlayerState->GetRoundWins(),
                    PvPGameState ? PvPGameState->GetRoundWinsToWin() : 3,
                    PvPPlayerState->GetMatchKills(),
                    PvPPlayerState->GetMatchDeaths())));
        }
    }

    if (PvPGameState)
    {
        const bool bIsRoundEnd = PvPGameState->GetRoundState() == EPvPARoundState::RoundEnd
            && PvPGameState->GetMatchPhase() == EPvPAMatchPhase::Playing;
        const bool bIsLobby = PvPGameState->GetMatchPhase() == EPvPAMatchPhase::Lobby;
        const bool bIsMatchEnd = PvPGameState->GetMatchPhase() == EPvPAMatchPhase::MatchEnd;

        ApplyLobbyInputMode(PlayerController, bIsLobby);

        if (AnnouncementPanel)
        {
            AnnouncementPanel->SetVisibility(bIsRoundEnd ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (LobbyPanel)
        {
            LobbyPanel->SetVisibility(bIsLobby ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (LobbyControlsPanel)
        {
            LobbyControlsPanel->SetVisibility(bIsLobby ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (MatchResultPanel)
        {
            MatchResultPanel->SetVisibility(bIsMatchEnd ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (TimerText)
        {
            if (bIsLobby)
            {
                TimerText->SetText(FText::FromString(TEXT("Time: Waiting")));
            }
            else if (bIsMatchEnd)
            {
                TimerText->SetText(FText::FromString(FString::Printf(TEXT("Lobby In: %d"), PvPGameState->GetRemainingMatchEndTimeSeconds())));
            }
            else
            {
                TimerText->SetText(FText::FromString(FString::Printf(TEXT("Time: %d"), PvPGameState->GetRemainingRoundTimeSeconds())));
            }
        }

        if (RoundStateText)
        {
            RoundStateText->SetText(FText::FromString(
                FString::Printf(
                    TEXT("Phase: %s | Round: %s"),
                    *MatchPhaseToString(static_cast<uint8>(PvPGameState->GetMatchPhase())),
                    *RoundStateToString(static_cast<uint8>(PvPGameState->GetRoundState())))));
        }

        if (ResultText)
        {
            if (bIsRoundEnd)
            {
                ResultText->SetVisibility(ESlateVisibility::Visible);
                ResultText->SetText(FText::FromString(GetRoundResultText(PlayerController, PvPGameState)));
            }
            else
            {
                ResultText->SetVisibility(ESlateVisibility::Collapsed);
                ResultText->SetText(FText::GetEmpty());
            }
        }

        if (NextRoundText)
        {
            if (bIsRoundEnd)
            {
                NextRoundText->SetVisibility(ESlateVisibility::Visible);
                NextRoundText->SetText(FText::FromString(
                    FString::Printf(TEXT("Next Round In: %d"), PvPGameState->GetRemainingRoundEndTimeSeconds())));
            }
            else
            {
                NextRoundText->SetVisibility(ESlateVisibility::Collapsed);
                NextRoundText->SetText(FText::GetEmpty());
            }
        }

        if (LobbyStatusText)
        {
            LobbyStatusText->SetText(FText::FromString(bIsLobby ? GetLobbyStatusText(PvPGameState) : FString()));
        }

        if (LobbyReadyButton)
        {
            LobbyReadyButton->SetVisibility(bIsLobby ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (LobbyReadyButtonText)
        {
            const APvPArenaPlayerState* LocalPlayerState = PlayerController->GetPlayerState<APvPArenaPlayerState>();
            const bool bReadyForLobbyStart = LocalPlayerState && LocalPlayerState->IsReadyForLobbyStart();
            LobbyReadyButtonText->SetText(FText::FromString(bReadyForLobbyStart ? TEXT("Cancel Ready") : TEXT("Ready")));
        }

        if (MatchResultTitleText)
        {
            if (bIsMatchEnd)
            {
                MatchResultTitleText->SetVisibility(ESlateVisibility::Visible);
                MatchResultTitleText->SetText(FText::FromString(GetMatchResultText(PlayerController, PvPGameState)));
            }
            else
            {
                MatchResultTitleText->SetVisibility(ESlateVisibility::Collapsed);
                MatchResultTitleText->SetText(FText::GetEmpty());
            }
        }

        if (MatchResultSummaryText)
        {
            if (bIsMatchEnd)
            {
                MatchResultSummaryText->SetVisibility(ESlateVisibility::Visible);
                MatchResultSummaryText->SetText(FText::FromString(GetMatchSummaryText(PlayerController, PvPGameState)));
            }
            else
            {
                MatchResultSummaryText->SetVisibility(ESlateVisibility::Collapsed);
                MatchResultSummaryText->SetText(FText::GetEmpty());
            }
        }
    }
}

void UPvPArenaHUDWidget::ApplyLobbyInputMode(APlayerController* PlayerController, bool bEnableLobbyInput)
{
    if (!PlayerController || bLobbyInputModeActive == bEnableLobbyInput)
    {
        return;
    }

    if (bEnableLobbyInput)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetHideCursorDuringCapture(false);
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PlayerController->SetInputMode(InputMode);
        PlayerController->bShowMouseCursor = true;
    }
    else
    {
        FInputModeGameOnly InputMode;
        PlayerController->SetInputMode(InputMode);
        PlayerController->bShowMouseCursor = false;
    }

    bLobbyInputModeActive = bEnableLobbyInput;
}

void UPvPArenaHUDWidget::HandleLobbyReadyButtonClicked()
{
    APvPArenaPlayerController* PvPPlayerController = Cast<APvPArenaPlayerController>(GetOwningPlayer());
    if (!PvPPlayerController)
    {
        return;
    }

    PvPPlayerController->ToggleLobbyReady();
}

void UPvPArenaHUDWidget::RefreshCrosshairVisibility()
{
    if (!RangedCrosshairOverlay)
    {
        return;
    }

    const APlayerController* PlayerController = GetOwningPlayer();
    const APvPArenaCharacter* Character = PlayerController ? Cast<APvPArenaCharacter>(PlayerController->GetPawn()) : nullptr;
    RangedCrosshairOverlay->SetVisibility(BuildRangedCrosshairVisibilityState(Character));
}

FString UPvPArenaHUDWidget::GetRoundResultText(const APlayerController* PlayerController, const APvPArenaGameState* GameState) const
{
    if (!PlayerController || !GameState)
    {
        return TEXT("Unknown");
    }

    const APvPArenaPlayerState* LocalPlayerState = PlayerController->GetPlayerState<APvPArenaPlayerState>();
    if (!LocalPlayerState)
    {
        return TEXT("Unknown");
    }

    const int32 LocalKills = LocalPlayerState->GetRoundKills();
    int32 HighestOpponentKills = TNumericLimits<int32>::Min();
    bool bFoundOpponent = false;

    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPState = Cast<APvPArenaPlayerState>(PlayerState);
        if (!PvPState || PvPState == LocalPlayerState)
        {
            continue;
        }

        HighestOpponentKills = FMath::Max(HighestOpponentKills, PvPState->GetRoundKills());
        bFoundOpponent = true;
    }

    if (!bFoundOpponent)
    {
        return TEXT("Pending");
    }

    if (LocalKills > HighestOpponentKills)
    {
        return TEXT("Victory");
    }

    if (LocalKills < HighestOpponentKills)
    {
        return TEXT("Defeat");
    }

    return TEXT("Draw");
}

FString UPvPArenaHUDWidget::GetLobbyStatusText(const APvPArenaGameState* GameState) const
{
    int32 ConnectedPlayers = 0;
    int32 ReadyPlayers = 0;
    if (GameState)
    {
        for (APlayerState* PlayerState : GameState->PlayerArray)
        {
            const APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
            if (!PvPPlayerState)
            {
                continue;
            }

            ++ConnectedPlayers;
            if (PvPPlayerState->IsReadyForLobbyStart())
            {
                ++ReadyPlayers;
            }
        }
    }

    const int32 CountdownSeconds = GameState ? GameState->GetRemainingLobbyCountdownSeconds() : 0;
    if (CountdownSeconds > 0)
    {
        return FString::Printf(
            TEXT("Players Ready: %d / %d\nMatch starts in: %d\nFirst to %d round wins takes the match."),
            ReadyPlayers,
            ConnectedPlayers,
            CountdownSeconds,
            GameState ? GameState->GetRoundWinsToWin() : 3);
    }

    return FString::Printf(
        TEXT("Players Ready: %d / %d\nNeed all players ready to begin.\nFirst to %d round wins takes the match."),
        ReadyPlayers,
        ConnectedPlayers,
        GameState ? GameState->GetRoundWinsToWin() : 3);
}

FString UPvPArenaHUDWidget::GetMatchResultText(const APlayerController* PlayerController, const APvPArenaGameState* GameState) const
{
    if (!PlayerController || !GameState)
    {
        return TEXT("Match Complete");
    }

    const APvPArenaPlayerState* LocalPlayerState = PlayerController->GetPlayerState<APvPArenaPlayerState>();
    const APvPArenaPlayerState* MatchWinner = GameState->GetMatchWinner();
    if (!LocalPlayerState || !MatchWinner)
    {
        return TEXT("Match Complete");
    }

    return MatchWinner == LocalPlayerState ? TEXT("Final Victory") : TEXT("Final Defeat");
}

FString UPvPArenaHUDWidget::GetMatchSummaryText(const APlayerController* PlayerController, const APvPArenaGameState* GameState) const
{
    if (!PlayerController || !GameState)
    {
        return TEXT("Returning to lobby...");
    }

    const APvPArenaPlayerState* LocalPlayerState = PlayerController->GetPlayerState<APvPArenaPlayerState>();
    if (!LocalPlayerState)
    {
        return TEXT("Returning to lobby...");
    }

    int32 HighestOpponentRoundWins = 0;
    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPState = Cast<APvPArenaPlayerState>(PlayerState);
        if (!PvPState || PvPState == LocalPlayerState)
        {
            continue;
        }

        HighestOpponentRoundWins = FMath::Max(HighestOpponentRoundWins, PvPState->GetRoundWins());
    }

    return FString::Printf(
        TEXT("Final Rounds: %d - %d\nK / D: %d / %d\nReturning to lobby in: %d"),
        LocalPlayerState->GetRoundWins(),
        HighestOpponentRoundWins,
        LocalPlayerState->GetMatchKills(),
        LocalPlayerState->GetMatchDeaths(),
        GameState->GetRemainingMatchEndTimeSeconds());
}

FString UPvPArenaHUDWidget::RoundStateToString(uint8 RoundStateValue)
{
    switch (static_cast<EPvPARoundState>(RoundStateValue))
    {
    case EPvPARoundState::Playing:
        return TEXT("Playing");
    case EPvPARoundState::RoundEnd:
        return TEXT("RoundEnd");
    case EPvPARoundState::SuddenDeath:
        return TEXT("SuddenDeath");
    default:
        return TEXT("Unknown");
    }
}

FString UPvPArenaHUDWidget::MatchPhaseToString(uint8 MatchPhaseValue)
{
    switch (static_cast<EPvPAMatchPhase>(MatchPhaseValue))
    {
    case EPvPAMatchPhase::Lobby:
        return TEXT("Lobby");
    case EPvPAMatchPhase::Playing:
        return TEXT("Playing");
    case EPvPAMatchPhase::MatchEnd:
        return TEXT("MatchEnd");
    default:
        return TEXT("Unknown");
    }
}
